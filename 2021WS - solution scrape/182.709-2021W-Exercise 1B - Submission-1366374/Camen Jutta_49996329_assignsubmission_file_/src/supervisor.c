/**
@file supervisor.c
@author Jutta Camen <e01452468@student.tuwien.ac.at>
@date 12.11.2021

@brief Main program supervisor,
which sets up shared memory and semaphore.
@details creates shared memory for circular buffer, to which
generators write their solutions. If there is a valid solution with 0 edges
to be removed (graph is acyclic), supervisor sets quit=1 (also if there 
is a signal to quit)

synopsis: supervisor
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //getopt, ftruncate

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <sys/types.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

#include <signal.h>
#include "configuration.h"

//! @details if quit == 1, supervisor and generators stop
static volatile sig_atomic_t quit = 0;

/**
@author Jutta Camen <e01452468@student.tuwien.ac.at>
@date 14.11.2021
@brief if there is a signal to quit, set quit=1
@param signal type
@details global variables: quit
*/
static void handle_signal(int signal){
    quit=1;
}

/**
@author Jutta Camen <e01452468@student.tuwien.ac.at>
@date 12.11.2021
@details program entry point, creates shared memory for circular buffer, to which
generators write their solutions. If there is a valid solution with 0 edges
to be removed (graph is acyclic), supervisor sets quit=1 (also if there 
is a signal to quit), global variables: quit

@brief program entry point

@param argc: count of strings in argv
@param argv: strings
@return returns EXIT_SUCCESS on success, else EXIT_FAILURE
*/
int main(int argc, char *argv[]) {
    char *prog_name = argv[0];
    if(argc>1){
        fprintf(stderr, "[%s:%d] ERROR: wrong input. synopsis: supervisor\n", prog_name, __LINE__);
        exit(EXIT_FAILURE);
    }
    
    //bei cancel signal wird quit auf 1 gesetzt //signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler=handle_signal;
    if (sigaction(SIGINT, &sa, NULL) == -1){
        fprintf(stderr, "[%s:%d] ERROR: sigaction (SIGINT) failed : %s\n", prog_name, __LINE__, strerror(errno));
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1){
        fprintf(stderr, "[%s:%d] ERROR: sigaction (SIGTERM) failed : %s\n", prog_name, __LINE__, strerror(errno));
    }

    //OPEN SHARED MEMORY, SEMAPHORE
    int shmfd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, 0600); //(0600) User kann lesen und schreiben
    if (shmfd == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not create shared memory : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct myshm)) < 0){
        fprintf(stderr, "[%s:%d] ERROR: ftruncate failed : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // map shared memory object:
    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED){
        fprintf(stderr, "[%s:%d] ERROR: mmap failed : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    myshm->readPos = 0;
    myshm->writePos = 0;
    myshm->quit = 0;
    
    // semaphore
    sem_t *sem_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, MAX_DATA);
    if (sem_write==SEM_FAILED){
        fprintf(stderr, "[%s:%d] ERROR: could not create semaphore write : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    sem_t *sem_read = sem_open(SEM_READ, O_CREAT | O_EXCL, 0600, 0);
    if (sem_read==SEM_FAILED){
        fprintf(stderr, "[%s:%d] ERROR: could not create semaphore read : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    sem_t *sem_excl = sem_open(SEM_EXCL, O_CREAT | O_EXCL, 0600, 1);
    if (sem_excl==SEM_FAILED){
        fprintf(stderr, "[%s:%d] ERROR: could not create semaphore exclusive : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    //terminate if solution with 0 edges
    //notify generators to terminate beforehand, unlink all shared memory, then exit
    int minimum=MAX_REMOVE+1;
    int edgesRemove[MAX_REMOVE*2];
    printf("[%s] Program start successful!\n", prog_name);
    while(quit==0){
        if (sem_wait(sem_read) == -1){ //warte dass ein generator schreibt
            fprintf(stderr, "[%s] ERROR: sem_wait(sem_read) failed : %s\n", prog_name, strerror(errno));
            break;
        } 
        int readPos = myshm->readPos;
        
        if (myshm->data[readPos][0]<minimum){
            minimum=myshm->data[readPos][0];
            
            if (minimum==0){
                printf("[%s] The graph is acyclic!\n", prog_name);
                if (sem_post(sem_write) == -1){ // gib ein schreiben frei
                    fprintf(stderr, "[%s:%d] ERROR: sem_post(sem_write) failed : %s\n", prog_name, __LINE__, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                break;
            }
            printf("[%s] Solution with %d edges:", prog_name, minimum);
            
            for (int i=0; i<minimum; i++){
                edgesRemove[2*i]=myshm->data[readPos][2*i+1];
                edgesRemove[2*i+1]=myshm->data[readPos][2*i+2];
                printf(" %d-%d", edgesRemove[2*i], edgesRemove[2*i+1]);
            }
            
            printf("\n");
        }
        
        myshm->readPos = (readPos+1)%MAX_DATA;
        
        if (sem_post(sem_write) == -1){ // gib ein schreiben frei
            fprintf(stderr, "[%s:%d] ERROR: sem_post(sem_write) failed : %s\n", prog_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    myshm->quit=1;
    
    //CLOSE SHARED MEMORY, SEMAPHORE
    // unmap shared memory:
    if (munmap(myshm, sizeof(*myshm)) == -1){
        fprintf(stderr, "[%s:%d] ERROR: munnmap shared memory failed: %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // close shared memory
    if (close(shmfd) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not close shared memory : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // remove shared memory object:
    if (shm_unlink(SHM_NAME) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not unlink shared memory : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // close semaphores
    if (sem_close(sem_write) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not close semaphore_write : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_read) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not close semaphore_read : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_excl) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not close semaphore_excl : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // remove semaphores
    if (sem_unlink(SEM_WRITE) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not unlink semaphore_write : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem_unlink(SEM_READ) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not unlink semaphore_read : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem_unlink(SEM_EXCL) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not unlink semaphore_exclusive : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}
