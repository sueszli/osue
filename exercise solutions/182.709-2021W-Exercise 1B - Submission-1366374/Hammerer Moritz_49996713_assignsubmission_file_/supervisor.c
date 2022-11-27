#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include "supervisor.h"


/**
 * @name: supervisor
 * @author: Moritz Hammerer, 11902102
 * @date: 08.11.2021
 * 
 * @brief Creating Shared Memory and Semaphores for generators and output each shorter answer
 * 
 * @details Creating Shared Memory and Semaphores for generators and initialises the shm. The generators
 * are writting their solutions to the circular buffer and the supervisor keeps track and outputs the answer.
 * 
 */

volatile sig_atomic_t quit = 0;
char *myprog;

/**
 * @brief Acts as Signalhandler and sets quit to one
 *  
 * @param signal    unused argument
 */
void handle_signal(int signal)
{
    quit = 1;
}

/**
 * @brief Gives out custom error message followed by errno and exits program
 * 
 * @details Takes an Char* with changable text and outputs it together with the name of the program and the corresponding errno.
 * Afterwards the program gets closed with ERROR_FAILURE
 *  
 * @param text      Char* Short text to futher explain the problem
 */
void errorOut(char* text){
    fprintf(stderr, "[%s] %s: %s\n", myprog, text, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Creating Shared Memory and Semaphores for generators and output each shorter answer
 *  
 * @details Creating Shared Memory and Semaphores for generators and initialises the shm. The generators
 * are writting their solutions to the circular buffer and the supervisor keeps track and outputs the answer.
 */
int main(int argc, char* argv[]) {
    myprog = argv[0];
    if (argc >= 2){
        errno = E2BIG;
        errorOut("Usage error. Dont start with Arguments");
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //Starting
    
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1){
        errorOut("Creating/Opening of '/shm11902102' failed");
    }
    if (ftruncate(shmfd, sizeof(struct myshm)) < 0){
        errorOut("Truncate of '/shm11902102' failed");
    }

    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED){
        errorOut("Mapping of '/shm11902102' failed");
    }

    myshm->state = 1;
    myshm->wr_pos = 0;
    myshm->currentMin = MAX_EDGES;

    sem_t *writeSem = sem_open(WRITESEM, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    sem_t *readSem = sem_open(READSEM, O_CREAT | O_EXCL, 0600, 0); 
    sem_t *permSem = sem_open(PERMSEM, O_CREAT | O_EXCL, 0600, 1); 

    if (writeSem == SEM_FAILED || readSem == SEM_FAILED || permSem == SEM_FAILED )
    {
        errorOut("Opening of Semaphores failed");
    }
    
    //critical
    int rd_pos = 0;
    int minEdges = MAX_EDGES + 1;
    while (!quit) {
        sem_wait(readSem);
        struct dataEntry val = myshm->data[rd_pos];
        
        rd_pos += 1;
        rd_pos %= BUFFER_SIZE;
        sem_post(writeSem);

        if (val.count < minEdges && !quit){
            if (val.count == 0)
            {
                myshm->currentMin = 0;
                fprintf(stdout, "The graph is 3 colorable!\n");
                break;
            } else{
                minEdges = val.count;
                myshm->currentMin = minEdges;
                fprintf(stdout, "Solution with %d edges:", minEdges);
                for (int i = 0; i < minEdges; i++)
                {
                    fprintf(stdout, " %d-%d", val.edges[i].u, val.edges[i].v);
                }
                fprintf(stdout, "\n");
            }
        }

    }

    
    //Cleanup
    myshm->state = 0;

    sem_close(writeSem);
    sem_close(readSem);
    sem_close(permSem);

    sem_unlink(WRITESEM);
    sem_unlink(READSEM);
    sem_unlink(PERMSEM);

    munmap(myshm, sizeof(*myshm));
    close(shmfd);
    shm_unlink(SHM_NAME);


    exit(EXIT_SUCCESS);
}