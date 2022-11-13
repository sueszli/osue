/**
 * @file supervisor.c
 * @author Fodor Francesca Diana, 11808223
 * @date 08.11.2021
 * @brief This program reads solutions for a set of edges to remove from a graph so that node adjacent
 * nodes share the same color. It then prints them in the console if the number of edges is less than
 * the previously printed solution - a better solution is found.
 * If the number of edges read is zero it prints a message that the graph is acyclic and exits.
 * @details The supervisor program opens a shared memory file and expands its size so that 20
 * solutions can be written into the file. It then opens three semaphores to coordinate
 * communication with one or more generators. Everytime a solution is read that is better than the
 * first one, it is printed into the console. A solution is better than another if the len value,
 * of an entry is smaller than the previous smallest. If len is zero the smallest possible solution
 * to the 3-coloring problem has been found. If the graph is acyclic then the program will print this
 * into the console and will terminate the program by sending a signal to the generators to stop generating
 * solutions and to exit. After all generators have exited, the program will unlink the shared
 * memory file and all semaphores that have been created and will also exit.
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>
#include "sharedfunctions.h"

#define ERR_EXIT(msg) fprintf(stderr, "[%s] %s error: %s\n", argv[0], msg, strerror(errno)); exit(EXIT_FAILURE);
#define USER_ERROR(msg) fprintf(stderr, "%s\n", msg); exit(EXIT_FAILURE);


struct myshmentry *myshmentry;


/**
 * Async singal handler function
 * @brief this function handles an incoming signal
 * @details once a signal has been caught, the state of the struct myshmentry is set to 1. Triggering the while loop to end
 * and thus the program to be interrupted.
 * @param signal incoming signal
 */
void handle_signal(int signal) {
    //interrupt = 1;
    myshmentry->state = 1;
}

int main(int argc, char *argv[]) {

    //signal handling setup
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_handler = handle_signal;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);

    //create shared memory object
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        ERR_EXIT("shm_open");
    }

    //set size of shm
    if (ftruncate(shmfd, sizeof(struct myshmentry)) < 0) {
        close(shmfd);
        shm_unlink(SHM_NAME);
        ERR_EXIT("ftruncate");
    }

    //map memory to virtual space
    myshmentry = mmap(NULL, sizeof(*myshmentry), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshmentry == MAP_FAILED) {
        close(shmfd);
        shm_unlink(SHM_NAME);
        ERR_EXIT("mmap");
    }

    //create semaphores
    sem_t *sem_pst = sem_open(SEM_PST, O_CREAT | O_EXCL, 0600, MAX_LENGTH);
    if (sem_pst == SEM_FAILED) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("sem_open SEM_PST");
    }
    sem_t *sem_rd = sem_open(SEM_RD, O_CREAT | O_EXCL, 0600, 0);
    if (sem_rd == SEM_FAILED) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("sem_open SEM_RD");
    }
    sem_t *sem_sync = sem_open(SEM_SYNC, O_CREAT | O_EXCL, 0600, 1);
    if (sem_sync == SEM_FAILED) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("sem_open SEM_SYNC");
    }

    memset(myshmentry, 0, sizeof(*myshmentry));

    //init solution struct
    color3 bestSolution;
    bestSolution.numberOfEdges = 8;


    //wait for generator solutions as long as not interrupted
    while (myshmentry->state != 1) {
        if (sem_wait(sem_rd) < 0) {
            if (errno == EINTR) {
                break;
            }
            shm_unlink(SHM_NAME);
            sem_close_all(2, sem_rd, sem_pst);
            sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
            ERR_EXIT("sem_wait");
        }

        if (myshmentry->solution[myshmentry->readIndex].numberOfEdges == 0) {
            printf("The graph is 3-colorable!\n");
            myshmentry->state = 1;
            break;
        } else if (myshmentry->solution[myshmentry->readIndex].numberOfEdges < bestSolution.numberOfEdges) {
            bestSolution = myshmentry->solution[myshmentry->readIndex];
            printf("Solution with %d edges: ", bestSolution.numberOfEdges);
            for (int i = 0; i < bestSolution.numberOfEdges * 2; i++) {
                if (bestSolution.edges[i] == 0) {
                    if (bestSolution.edges[i + 1] == 0) {
                        break;
                    }
                }
                if (i % 2 == 0) {
                    printf("%d-", bestSolution.edges[i]);
                } else {
                    printf("%d ", bestSolution.edges[i]);
                }
            }
            printf("\n");
        }

        //recalculate the read index inside the circular buffer and check if buffer is empty
        myshmentry->readIndex = (myshmentry->readIndex + 1) % MAX_LENGTH;
        if (sem_post(sem_pst) < 0) {
            shm_unlink(SHM_NAME);
            sem_close_all(2, sem_rd, sem_pst);
            sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
            ERR_EXIT("sem_post");
        }
    }

    //TODO: maybe use SEM_FINISH to wait for all generators to complete

    //cleanup shared memory object
    if (munmap(myshmentry, sizeof(*myshmentry)) == -1) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("munmap");
    }

    //close SHM file descriptor
    if (close(shmfd) == -1) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("close");
    }

    //remove SHM
    if (shm_unlink(SHM_NAME) == -1) {
        ERR_EXIT("shm_unlink");
    }

    //cleanup semaphores
    if (sem_close(sem_rd) == -1) {
        shm_unlink(SHM_NAME);
        sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
        ERR_EXIT("sem_close SEM_RD");
    }
    if (sem_close(sem_pst) == -1) {
        shm_unlink(SHM_NAME);
        sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
        ERR_EXIT("sem_close SEM_PST");
    }
    if (sem_close(sem_sync) == -1) {
        shm_unlink(SHM_NAME);
        sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
        ERR_EXIT("sem_close SEM_SYNC");
    }

    //remove semaphores
    if (sem_unlink(SEM_RD) == -1) {
        ERR_EXIT("sem_unlink SEM_RD");
    }
    if (sem_unlink(SEM_PST) == -1) {
        ERR_EXIT("sem_unlink SEM_RD");
    }
    if (sem_unlink(SEM_SYNC) == -1) {
        ERR_EXIT("sem_unlink SEM_SYNC");
    }

    return 0;
}