/**
 * @file supervisor.c
 * @author Peter Haraszti <Matriculation Number: 12019844>
 * @date 08.11.2021
 *
 * @brief supervisor is a program that supervises a number of generator programs, that find a solution for the 3coloring problem
 * @details First, the supervisor sets up a shared memory with a circular buffer. Then, it waits for the generator programs to start, that write solutions to the 3coloring problem into the circular buffer.
 * The supervisor reads from the circular buffer. If a solution is in the circular buffer, that is better than the solutions found so far, the supervisor prints it to stdout.
 * If a solution contains no edges, the graph is 3 colorable. In that case, the supervisor stops the generators and terminates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>
#include <semaphore.h>

#include "structs.h"

#define SH_NAME_12019844 "/12019844myshm"

#define SEM_FREE_12019844 "/12019844sem_f"
#define SEM_USED_12019844 "/12019844sem_u"
#define SEM_WRITE_12019844 "/12019844sem_write"

struct myshm *shm;
int shmfd;
int bestSolutionSize = INT_MAX;
struct Solution bestSolution;

sem_t *free_sem;
sem_t *used_sem;
sem_t *write_sem;
int rd_pos = 0;

char *programname = NULL;

/**
 * @brief Opens the shared memory
 * @details openSHM opens the shared memory, if no shared memory was created yet, it creates it. In case the operation fails, the program exits with an error. The file descriptor is returned.
 * @param void
 * @return The file descriptor of the shared memory is returned
 */
int openSHM(void) {     // create and/or open the shared memory object:
    shmfd = shm_open(SH_NAME_12019844, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "[%s]: Couldn't open shared memory\n", programname);
        exit(EXIT_FAILURE);
    }
    return shmfd;
}

/**
 * @brief Sets the size of the shared memory
 * @details truncateSHM sets the size of the shared memory. If the operation fails, the program exits with an error.
 * @param void
 * @return void
 */
void truncateSHM(void) {
    if (ftruncate(shmfd, sizeof(struct myshm)) < 0) {
        fprintf(stderr, "[%s]: Couldn't set size of shared memory\n", programname);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Maps the shared memory
 * @details mapSHM maps the shared memory. If the operation fails, the program exits with an error.
 * @param void
 * @return void
 */
void mapSHM(void) {
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm == MAP_FAILED) {
        fprintf(stderr, "[%s]: shared memory: MAP_FAILED\n", programname);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Unmaps the shared memory
 * @details unmapSHM unmaps the shared memory. If the operation fails, the program exits with an error.
 * @param void
 * @return void
 */
void unmapSHM(void) {
    if (munmap(shm, sizeof(*shm)) == -1) {
        fprintf(stderr, "[%s]: shared memory: munmap failed\n", programname);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Unlinks the shared memory
 * @details unlinkSHM unlinks the shared memory. If the operation fails, the program exits with an error.
 * @param void
 * @return void
 */
void unlinkSHM(void) {    // remove shared memory object:
    if (shm_unlink(SH_NAME_12019844) == -1) {
        fprintf(stderr, "[%s]: shared memory: unlink failed\n", programname);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints a solution
 * @details Prints the number of edges and the edges themselves that need to be removed to make the graph 3colorable.
 * @param sol: a struct Solution to be printed
 * @return void
 */
void printSolution(struct Solution sol) {
    printf("Supervisor: Found solution of size %d; Solution: ", sol.numEdges);
    for (int i = 0; i < sol.numEdges; i++) {
        printf("%d - %d; ", sol.edges[i].from, sol.edges[i].to);
    }
    printf("\n");
}

/**
 * @brief Handling of SIGINT and SIGTERM
 * @details In case a signal SIGINT or SIGTERM occurs, the generators get stopped and the semaphores and the shared memory closed/unmapped/unlinked. After that, the program exits.
 * @param signal
 * @return void
 */
void handle_signal(int signal) {
    shm->stop = 1;
    sem_close(free_sem);
    sem_close(used_sem);
    sem_close(write_sem);
    sem_unlink(SEM_FREE_12019844);
    sem_unlink(SEM_USED_12019844);
    sem_unlink(SEM_WRITE_12019844);
    close(shmfd);
    unmapSHM();
    unlinkSHM();
    _exit(1);
}

/**
 * @brief Sets up Signal Handling
 * @details If a SIGTERM or a SIGINT occurs, handle_signal is called
 * @param void
 * @return void
 */
void initSignalHandling(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/**
 * @brief Initializes the shared memory
 * @details openSHM(), truncateSHM() and mapSHM() are called
 * @param void
 * @details void
 */
void initSharedMem(void) {
    shmfd = openSHM();
    truncateSHM();
    mapSHM();
}

/**
 * @brief Initializes the semaphores
 * @details free_sem, used_sem and write_sem are opened or created if they don't exist. If the operation fails, the program quits with an error.
 * @param void
 * @return void
 */
void initSemaphores(void) {
    free_sem = sem_open(SEM_FREE_12019844, O_CREAT | O_EXCL, 0600, BUFFER_LENGTH);
    if (free_sem == NULL) {
        fprintf(stderr, "[%s]: Opening free_sem failed\n", programname);
        exit(EXIT_FAILURE);
    }
    used_sem = sem_open(SEM_USED_12019844, O_CREAT | O_EXCL, 0600, 0);
    if (used_sem == NULL) {
        fprintf(stderr, "[%s]: Opening used_sem failed\n", programname);
        exit(EXIT_FAILURE);
    }

    write_sem = sem_open(SEM_WRITE_12019844, O_CREAT | O_EXCL, 0600, 1);
    if (write_sem == NULL) {
        fprintf(stderr, "[%s]: Opening write_sem failed\n", programname);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Starts the generator processes
 * @details The number of processes and the graphs to be checked can be specified in the for loop.
 * @param void
 * @return void
 */
void startGenerators(void) {
    for (int i = 0; i < 10; i++) {
        system("./generator 0-1 0-2 0-3 1-28 1-29 2-30 2-31 3-32 3-33 4-6 4-14 4-16 5-7 5-15 5-17 6-7 6-18 7-19 8-9 8-12 8-23 9-13 9-22 10-15 10-19 10-25 11-14 11-18 11-24 12-17 12-27 13-16 13-26 14-23 15-22 16-21 17-20 18-21 19-20 20-31 21-30 22-33 23-32 24-27 24-29 25-26 25-29 26-28 27-28 30-33 31-32 &");
        //system("./generator 0-1 0-3 0-4 1-2 1-3 1-4 1-5 2-4 2-5 3-4 4-5 &");
        //system("./generator 0-1 0-3 0-4 1-2 1-3 1-5 2-4 2-5 3-4 4-5 &");
    }
}

/**
 * @brief Cleans up resources after the program is done
 * @details The shared memory is unmaped, closed and unlinked. The semaphores are closed and unlinked.
 * @param void
 * @return void
 */
void cleanup(void) {
    unmapSHM();
    close(shmfd);
    unlinkSHM();
    sem_close(free_sem);
    sem_close(used_sem);
    sem_close(write_sem);
    sem_unlink(SEM_FREE_12019844);
    sem_unlink(SEM_USED_12019844);
    sem_unlink(SEM_WRITE_12019844);
}

/**
 * @brief Main method of supervisor
 * @details First, signal handling, the shared memory and the semaphores are set up.
 * While there is no empty solution (the graph is not 3colorable), solutions are read from the circular buffer.
 * If the new solution is better than the previously found ones, it is stored in bestSolution and printed using the function printSolution()
 * If it turns out that the graph is 3colorable, the generators are stopped, the resources are cleaned up u
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS, if the program isn't stopped before that
 */
int main(int argc, char **argv) {
    programname = argv[0];

    initSignalHandling();
    initSharedMem();
    initSemaphores();

    //startGenerators();

    // Search for the best solution
    while (bestSolutionSize != 0) {

        // Read from buffer
        sem_wait(used_sem);
        struct Solution sol = shm->buff[rd_pos];
        //printf("Supervisor: Read from buffer at position [%d]\n", rd_pos);
        sem_post(free_sem);
        rd_pos += 1;
        rd_pos %= BUFFER_LENGTH;

        // Check if the read solution is better than the current best
        if (sol.numEdges < bestSolutionSize) {
            bestSolutionSize = sol.numEdges;
            bestSolution = sol;
            printSolution(bestSolution);
        }
    }
    printf("The graph is 3-colorable!\n");
    shm->stop = 1;
    cleanup();

    return EXIT_SUCCESS;
}

