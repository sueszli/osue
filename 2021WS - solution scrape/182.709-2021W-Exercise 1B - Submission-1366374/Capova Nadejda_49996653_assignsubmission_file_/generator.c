/**
 * @file   supervisor.c
 * @author Nadejda Capova (11923550)
 * @date   26.10.2021
 *
 * @brief The generators report their solutions to the supervisor, which remembers the set with least edges
        generates random solutions
 *
 * @details The generator program takes a graph (set of edges) as input.
 * The program repeatedly generates a random solution and writes its result to the circular buffer.
 * It repeats this procedure until it is notified by the supervisor to terminate.
 * The number of nodes of the graph is implicitly provided through the indices in the edges.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include "utility.h"


static char *myprog;


/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variable: myprog
 */
static void usage(void) {
    fprintf(stderr, "Usage: %s  generator EDGE1... \n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief The entry point of the program.
 * @details This is the only function that executes the whole program. Takes a graph as an input.
 * 

 *
 * @param argc  - argument counter
 * @param argv - argument values stored in an array
 *
 * @return Returns 0 upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[]) {
    //Checking if there are enough arguments in argv
    myprog = argv[0];
    char *arguments[argc - optind];
    if (argc - optind != 0)  // check if there are positional arguments
    {
        for (size_t i = 0; i < argc - optind; i++) //stores given values
        {
            arguments[i] = argv[optind + i];
        }
    } else {
        usage();
    }


    int size = sizeof(arguments) / sizeof(arguments[0]); //count of edges given in stdin
    edgeNode edges[size];

    char *arg;
    int countVertex = 0; //take the highest value of vertex
    //extract the edges
    for (int i = 0; i < size; i++) {
        arg = arguments[i];
        char *end;
        int a = strtol(arg, &end, 10);
        if (errno != 0) {
            fprintf(stderr, "strtol failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (countVertex < a) {
            countVertex = a;
        }
        int b = strtol(++end, &end, 10);

        if (errno != 0) {
            fprintf(stderr, "strtol failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (end == arg) {
            fprintf(stderr, "No digits were found\n");
            exit(EXIT_FAILURE);
        }
        if (countVertex < b) {
            countVertex = b;
        }
        edges[i].a = a;
        edges[i].b = b;
    }


    //creates or opens a shsred memry object
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "[%s] shmfd failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // map shared memory object into memory
    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE,
                 MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED) {
        fprintf(stderr, "[%s] mmap failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }
    //Opens an existing named semaphore
    //If both O_CREAT and O_EXCL are set, then open fails if the specified
    //file already exists. This is guaranteed to never clobber an existing file.
    sem_t *sem_fill = sem_open(FILL, O_EXCL);
    sem_t *sem_empty = sem_open(EMPTY, O_EXCL);
    sem_t *sem_mutex = sem_open(MUTEX, O_EXCL);
    int wr_pos = 0; //writing position in shared memory

    if ((sem_fill == SEM_FAILED) || (sem_empty == SEM_FAILED) || (sem_mutex == SEM_FAILED)) {
        fprintf(stderr, "sem_open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int nMax = 2; //colors represented through numbers from 0 to 2
    int nMin = 0;

    //each index represents a vertex in the graph
    int coloringVertex [countVertex+1];


    while (1)
    {
        if(myshm->ifStop==0){
            break;
        }

        //random set color for each vertex
        srand(time(NULL)+8);
        for(int i =0; i<=countVertex; i++){
            coloringVertex[i]=rand()%(nMax-nMin+1)+nMin;
       
        }
         //assign each vertex with color (colors are saved in array coloringVertex)
         int nullElem=0;

        for(int i =0; i<size; i++){
            int numA = edges[i].a;
            edges[i].colorA=coloringVertex[numA];
            int numB = edges[i].b;
            edges[i].colorB=coloringVertex[numB];
            if(edges[i].colorA==edges[i].colorB){ //if the vertexes of one edge have the same color
                edges[i].toRemove=1;
                nullElem++;
            }else{
                edges[i].toRemove=0;
            }
        }
        //valid 3-colorable graph to the shared memory
        arrayEdge_t  newGraph;
        newGraph.countEdges = (nullElem);
        int counterNewGraph=0;
        for(int i =0; i<size; i++){
            if(edges[i].toRemove==1){ 
                newGraph.node[counterNewGraph]=edges[i];
                counterNewGraph++;
            }
        }

        newGraph.countEdges = nullElem;
        sem_wait(sem_empty);
        sem_wait(sem_mutex);
        myshm->result[wr_pos]=newGraph;
        wr_pos+=1;
        wr_pos%=MAX_DATA;
        sem_post(sem_mutex);
        sem_post(sem_fill);



    }

//Removes whole memory pages from the given space
    if (munmap(myshm, sizeof *(myshm)) == -1) {
        fprintf(stderr, "[%s] munmap failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }
    //cleanup shared memory
    if (close(shmfd) == -1) {
        fprintf(stderr, "[%s] close failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }

    //close semaphores
    sem_close(sem_fill);
    sem_close(sem_empty);
    sem_close(sem_mutex);

    return EXIT_SUCCESS;
}
