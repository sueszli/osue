/**
 * @file supervisor.c
 * @author Dimitar Dimitrov 11719773
 * @date 14.01.2021
 *
 * @brief Supervisor
 * The supervisor sets up the shared memory and the semaphores and initializes the circular buffer required
 * for the communication with the generators. It then waits for the generators to write solutions to the
 * circular buffer.
 *
 * Once initialization is complete, the supervisor reads the solutions from the circular buffer and remembers the best solution so far
 * Every time a better solution than the previous best solution is found, the supervisor writes the new solution to standard output.
 * If a generator writes a solution with 0 edges to the circular buffer, then the graph is 3-colorable and the supervisor terminates.
 * Otherwise the supervisor keeps reading results from the circular buffer until it receives a SIGINT or a SIGTERM signal.
 *
 * Before terminating, the supervisor notifies all generators that they should terminate as well. The supervisor then unlinks all shared resources and exits.
 *
 * SYNOPSIS: supervisor
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>

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
int signalled = 1;

void cleanup(void);

void exitSignal(){
myshm->notsolved = 0;
}

void errorExit(char* errorMessage)
{
	fprintf(stderr, errorMessage);
    fprintf(stderr, "\n");
    if(forCleaning) cleanup();
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = exitSignal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if(argc != 1) errorExit("Supervisor does not take arguments.");
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd < 0) errorExit("Could not open shared memory.");
    if (ftruncate(shmfd, sizeof(myshm)) < 0)errorExit("Could not truncate the shared memory.");
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED)errorExit("Could not map the shared memory.");
    myshm->notsolved = 1;
    for(int i =0; i<MAX_CLIENTS; i++)myshm->placeMarker[i]=-1;
    free_sem = sem_open(FREE_SEM, O_CREAT | O_EXCL, 0600, MAX_CLIENTS);
    if (free_sem == SEM_FAILED) errorExit("Opening sem has failed.");

    used_sem = sem_open(USED_SEM, O_CREAT | O_EXCL, 0600, 0);
    if (used_sem == SEM_FAILED) errorExit("Opening sem has failed.");

    mutex_sem = sem_open(MUTEX_SEM, O_CREAT | O_EXCL, 0600, 1);   
    if (mutex_sem == SEM_FAILED) errorExit("Opening sem has failed.");
    unsigned int bestSolution = MAX_DATA;
    unsigned int semPos = 0;
    while((myshm->notsolved) && signalled){
        if (sem_wait(used_sem) < 0)errorExit("Sem wait failed.");
        while(myshm->placeMarker[semPos]==-1){semPos++; semPos=semPos%MAX_CLIENTS;}
        int solutionLength = myshm->placeMarker[semPos];
        if(solutionLength == 0)
        {
            printf("The graph is 3-colorable!\n");
                myshm->notsolved = 0;
                break;
        }
        if(bestSolution>solutionLength){
            printf("Solution with %d edges: ", solutionLength);
            for (int i = 0; i < solutionLength; i++) printf( "%d-%d ", myshm->graphData[semPos][i][0], myshm->graphData[semPos][i][1]);
            printf("\n");
            bestSolution=solutionLength;
        }
        myshm->placeMarker[semPos]=-1;
        if (sem_post(free_sem)<0) errorExit("Sem post failed.");
        semPos=(semPos+1)%MAX_CLIENTS;
    }
    cleanup();
    return EXIT_SUCCESS;
}

void cleanup(void){
    forCleaning=0;
    if ((free_sem!=NULL) && (sem_close(free_sem) < 0)) errorExit("Close free_sem failed.");
    if ((used_sem!=NULL) && (sem_close(used_sem) < 0)) errorExit("Close used_sem failed.");
    if ((mutex_sem!=NULL) && (sem_close(mutex_sem) <0)) errorExit("Close used_sem failed.");
    if (sem_unlink(FREE_SEM) < 0) errorExit("Unlink free_sem failed.");
    if (sem_unlink(USED_SEM) < 0) errorExit("Unlink used_sem failed.");
    if (sem_unlink(MUTEX_SEM) < 0) errorExit("Unlink used_sem failed.");
    if ((myshm!=NULL) && (munmap(myshm, sizeof(myshm)) < 0)) errorExit("Unmap has failed.");
    if((shmfd>=0) && (close(shmfd) < 0)) errorExit("Close shm failed.");
    if (shm_unlink(SHM_NAME) < 0) errorExit("Unlink failed.");
}