/**
 * @file supervisor.c
 * @author Michael Blank 11909459 <e11909459@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief Consumer Program for feedback arc set problem.
 * 
 * @details This Program creates a shared memory and 3 semaphore.
 * It waits until a produces has written a solution into the circular buffer.
 * If this solution is better then all previous solutions its written to stdout.
 * This continues until a solution with 0 edge removals have been found or the process
 * terminates.
 * In case of termination the supervisor will notify all the producers and then clean up
 * the shared memory and the semaphores.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include "arcSet.h"

/** Program quit flag
 * @brief The quit value is set to 1 if the program is terminating
 */
volatile sig_atomic_t quit;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details After writing the usage information the program will close with the EXIT_FAILURE value.
 * @param progName The name of the program.
 */
static void usage(char *progName) {
    fprintf(stderr, "Usage: %s\n", progName);
    exit(EXIT_FAILURE);
}

/**
 * Error helper function
 * @brief This function writes a message to stderr.
 * @details Given a message and a errnum this function will write the message and the corresponding error
 * message to stderr. Afterwards it will exit the program with the EXIT_FAILURE value.
 * @param progName The name of the program.
 */
static void error(char *progName, char *message ,int errnum) {
    fprintf(stderr, "%s: %s%s\n", progName, message, strerror(errnum));
    exit(EXIT_FAILURE);
}

/**
 * Singal handler function
 * @brief This function sets the quit flag to true
 * @details This function is called whenever the process recieves a SIGINT or SIGTERM signal.
 * By setting the quit flag to true the process will start cleaning up the shared memory and semaphores.
 * @param signal Signal id number
 */
static void handle_signal(int signal) {
    quit = 1;
}

/**
 * Program entry point.
 * @brief The program starts here. This function implements the functionality of the program.
 * @details This function parses all the arguments creates the shared memory and semaphores.
 * The shared memory is initialized with the structure of the circular buffer and the semaphores are set
 * to initial values. The function will then wait until a producer has written to the circular buffer
 * and compare the solution with the previously best solution. If the new solution is better its written
 * to stdout and is stored as the new best solution. If a solution with 0 edge removals is found the programm
 * starts cleaning up the shared memory and semaphores.
 * This is also the case when the quit flag is set to 1.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char* argv[]) {
    char *progName = argv[0];

    if (argc > 1)
    {
        usage(progName);
    }

    //Signal Interupt handleing
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //Shared Memory and Semaphore initialization
    int shmFd = shm_open(SHMNAME, O_CREAT | O_RDWR, S_IRWXU);
    if (shmFd == -1) {
        error(progName, "Shared Memory opening Error: ", errno);
    }
    
    if (ftruncate(shmFd, sizeof(struct myshm)) < 0) {
        close(shmFd);
        shm_unlink(SHMNAME);
        error(progName, "Shared Memory truncate Error: ", errno);
    }

    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (myshm == MAP_FAILED)
    {
        close(shmFd);
        shm_unlink(SHMNAME);
        error(progName, "Shared Memory mapping Error: ", errno);
    }

    sem_t *freeSem = sem_open(FREE_SEM, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    if(freeSem == SEM_FAILED) {
        munmap(myshm, sizeof(*myshm));
        close(shmFd);
        shm_unlink(SHMNAME);
        error(progName, "Semaphore opening Error: ", errno);
    }
    sem_t *usedSem = sem_open(USED_SEM, O_CREAT | O_EXCL, 0600, 0);
    if(usedSem == SEM_FAILED) {
        sem_close(freeSem);
        sem_unlink(FREE_SEM);
        munmap(myshm, sizeof(*myshm));
        close(shmFd);
        shm_unlink(SHMNAME);
        error(progName, "Semaphore opening Error: ", errno);
    }
    sem_t *writeSem = sem_open(WRITE_SEM, O_CREAT | O_EXCL, 0600, 1);
    if(writeSem == SEM_FAILED) {
        sem_close(usedSem);
        sem_unlink(USED_SEM);
        sem_close(freeSem);
        sem_unlink(FREE_SEM);
        munmap(myshm, sizeof(*myshm));
        close(shmFd);
        shm_unlink(SHMNAME);
        error(progName, "Semaphore opening Error: ", errno);
    }

    //Main Part    
    fprintf(stdout, "Setup done. Shm filesize: %ld bytes.\n", sizeof(struct myshm));

    struct bufEntry best;
    best.count = MAX_EDGES+1;

    while (!quit) {
        struct bufEntry current;

        memset(&current, 0, sizeof(current));
        
        sem_wait(usedSem);
        current = myshm->buf[myshm->rd_pos];
        myshm->rd_pos += 1;
        myshm->rd_pos %= sizeof(myshm->buf) / sizeof(struct bufEntry);
        sem_post(freeSem);

        if (current.count == 0 && !quit) {
            fprintf(stdout, "The graph is acyclic!\n");
            break;
        } else if (best.count > current.count && !quit) {
            best = current;

            fprintf(stdout, "Solution with %d edges: ", best.count);
            for (int i = 0; i < best.count; i++) {
                fprintf(stdout, "%d-%d ", best.edges[i].u, best.edges[i].v);
            }
            fprintf(stdout, "\n");
        }
    }

    myshm->state = 1;

    sem_close(writeSem);
    sem_close(usedSem);
    sem_close(freeSem);

    sem_unlink(WRITE_SEM);
    sem_unlink(USED_SEM);
    sem_unlink(FREE_SEM);
    
    munmap(myshm, sizeof(*myshm));
    close(shmFd);

    shm_unlink(SHMNAME);

    fprintf(stdout, "\nClosed the Shared Memory and Semaphores.\n");
    exit(EXIT_SUCCESS);
}