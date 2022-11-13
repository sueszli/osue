/**
 * @file supervisor.c
 * @author Dimitar Dimitrov 11719773
 * @date 14.01.2021
 *
 * @brief Generator
 *
 * The generator program takes an undirected graph (a set of edges) as input. Each positional argument is one edge.
 * An edge is specified by a string, with the indices of the two nodes it connects separated by a '-'. The number
 * of nodes of the graph is provided through the indices in the edges.
 * The program then repeatedly generates random solution till an acylic one is found. Each better than the previous
 * solution is written to the circular buff. The generator produces a debug output, describing the 3-coloring solution
 * and then writes it to the circular buff.
 * The generator terminates successfully if it finds an acyclic solution or the supervisor terminates.
 *
 * SYNOPSIS: generator EDGE1 EDGE2...
 * EXAMPLE: generator 0-1 1-2 2-3 3-4 4-5 5-6 6-7
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <limits.h>

#define SHM_NAME "/11719773_circularBuffer"
#define FREE_SEM "/11719773_free_sem"
#define USED_SEM "/11719773_used_sem"
#define MUTEX_SEM "/11719773_mutex_sem"
#define MAX_DATA (8)
#define MAX_CLIENTS (16)

struct circularBuffer {
    unsigned int notsolved;
    int placeMarker[MAX_CLIENTS];
    unsigned int graphData[MAX_CLIENTS][MAX_DATA][2];
} *myshm;

static sem_t *free_sem;
static sem_t *used_sem;
static sem_t *mutex_sem;

int shmfd=-1;
int forCleaning = 1;

void cleanup(void);

void errorExit(char* errorMessage)
{
	fprintf(stderr, errorMessage);
    fprintf(stderr,"\n");
    if(forCleaning) cleanup();
	exit(EXIT_FAILURE);
}

void checkEdgeArgument(char* arg) {
    int dash=0;
    for(int i=0; i<strlen(arg);i++)
    if(arg[i]<'0'||arg[i]>'9'){
        if(arg[i]=='-'){
            if(dash) errorExit("Invalid format for edge.");
            dash = 1;
        } else {
            errorExit("Invalid format for edge.");
        }
    }
}

int main(int argc, char *argv[])
{
    time_t t;
    if(argc == 1) errorExit("No graph given.");
    if(getopt(argc,argv, "") != -1) errorExit("The generator doesn't take arguments.");
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd < 0) errorExit("Could not open shared memory.");
    if (ftruncate(shmfd, sizeof(*myshm)) < 0)errorExit("Could not truncate the shared memory.");
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED)errorExit("Could not map the shared memory.");

    free_sem = sem_open(FREE_SEM, 0);
    if (free_sem == SEM_FAILED) errorExit("Error: sem_open() has failed!\n");

    used_sem = sem_open(USED_SEM, 0);
    if (used_sem == SEM_FAILED) errorExit("Error: sem_open() has failed!\n");

    mutex_sem = sem_open(MUTEX_SEM, 0);
    if (mutex_sem == SEM_FAILED) errorExit("Error: sem_open() has failed!\n");


    int* edgesX = malloc((argc-1)*sizeof(int));
    int* edgesY = malloc((argc-1)*sizeof(int));
    int verticeCount=0;
    for(int i=1;i<argc;i++){
        int edgeX=0, edgeY=0;
        int j = 0;
        while(argv[i][j]!='-'){
            edgeX=edgeX*10 + argv[i][j]-'0';
            j++;
        }j++;
        while(j<strlen(argv[i])){
            edgeY=edgeY*10 + argv[i][j]-'0';
            j++;
        }
        if(edgeX>verticeCount) verticeCount= edgeX;
        if(edgeY>verticeCount) verticeCount= edgeY;
        edgesX[i-1]=edgeX;
        edgesY[i-1]=edgeY;
        verticeCount++;
    }
    int* vertices = malloc(verticeCount*sizeof(int));
    
    int solutionXY[argc-1][2];
    srand((unsigned) time(&t));
    while(myshm->notsolved)
    {
        int solutionCount = 0;
        for(int i=0;i<verticeCount;i++)vertices[i]=rand()%3;
        for(int i=0;i<argc-1;i++)
        if(vertices[edgesX[i]]==vertices[edgesY[i]])
        {
            solutionXY[solutionCount][0]=edgesX[i];
            solutionXY[solutionCount][1]=edgesY[i];
            solutionCount++;
        }
        if(solutionCount<=MAX_DATA)
        {
            int clientSpace=0;
            if (sem_wait(free_sem) < 0) errorExit("Waiting free sem has failed.!");
            if (sem_wait(mutex_sem) < 0) errorExit("Waiting MUTEX has failed!\n");
            while(myshm->placeMarker[clientSpace]!=-1){ clientSpace++;clientSpace=clientSpace%MAX_CLIENTS;}
            myshm->placeMarker[clientSpace]=solutionCount;
            if (sem_post(mutex_sem) < 0) errorExit("Posting MUTEX has failed!\n");
            for (int i = 0; i < solutionCount; i++)
            {
                myshm->graphData[clientSpace][i][0] = solutionXY[i][0];
                myshm->graphData[clientSpace][i][1] = solutionXY[i][1];
            }
            if (sem_post(used_sem) < 0) errorExit("Posting used sem has failed.!");
        }
    }
    cleanup();
    return EXIT_SUCCESS;
}

void cleanup(void){
    forCleaning=0;
    if ((free_sem!=NULL) && (sem_close(free_sem) < 0)) errorExit("Close free_sem failed.");
    if ((used_sem!=NULL) && (sem_close(used_sem) < 0)) errorExit("Close used_sem failed.");
    if ((mutex_sem!=NULL) && (sem_close(mutex_sem) < 0)) errorExit("Close used_sem failed.");
    if ((myshm!=NULL) && (munmap(myshm, sizeof(myshm)) < 0)) errorExit("Unmap has failed.");
    if((shmfd>=0) && (close(shmfd) < 0)) errorExit("Close shm failed.");
}
