/**
 * @file generator.c
 * @author Tobias Grantner (12024016)
 * @brief This class implements the logic of a supervisor that reads all the feedback arc sets from a shared memory space with a circularbuffer
 *        that a generator generated and saves the shortest one and prints it until a feedback arc set with 0 edges is found or the program is interrupted.
 * @date 2021-11-11
 */

#include "shm.h"
#include "graph.h"
#include "circularbuffer.h"
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief sig_atomic_t variable, that is set to true if the program is interrupted by a SIGINT or a SIGTERM signal
 */
volatile sig_atomic_t quit = 0;

/**
 * @brief Handles the interruption of the program by a signal
 * 
 * @param signal The signal that interrupted the program
 */
void handle_signal(int signal)
{
    if(signal == SIGINT || signal == SIGTERM) {
        quit = 1;
    }
}


/**
 * @brief The name of the program. Is set at the beginning of the main-function and used to display the program name in error messages.
 */
static const char *program_name;

/**
 * @brief Displays the given error message after printing the program name.
 * 
 * @param message Error message to be displayed
 */
static void error(const char *message)
{
    fprintf(stderr, "[%s] ERROR: %s\n", program_name, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief Displays the given error message followed by the message from strerror(errno) after printing the program name.
 * 
 * @param message Error message to be displayed
 */
static void error_str(const char *message)
{
    fprintf(stderr, "[%s] ERROR: %s: %s\n", program_name, message, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief This is the main-function of the program and handles all the errors, parses the arguments, handles shared memory, semaphores and the program logic.
 * 
 * @param argc Number of passed arguments
 * @param argv Passed arguments
 * @return int Exit code
 */
int main(int argc, char **argv)
{
    program_name = argv[0];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);

    if (argc > 1)
    {
        error("This program does not accept arguments.");
    }

    // create and/or open the shared memory object:
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if(shmfd == -1) error_str("Opening shared memory failed");

    // set the size of the shared memory:
    if(ftruncate(shmfd, sizeof(shm_t)) < 0) error_str("ftruncate on shared memory failed");

    // map shared memory object:
    shm_t* shm;
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if(shm == MAP_FAILED) error_str("Shared memory mapping failed");

    shm->quit = 0;

    sem_t *sem_free = sem_open(CB_SEM_FREE, O_CREAT | O_EXCL, 0600, CB_MAX_SIZE);
    if(sem_free == SEM_FAILED) error_str("Semaphore open failed");

    sem_t *sem_used = sem_open(CB_SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    if(sem_used == SEM_FAILED) error_str("Semaphore open failed");

    graph_t smallest = { .count = GRAPH_MAX_EDGE_COUNT + 1 };
    graph_t current = smallest;

    while(!quit)
    {
        if(sem_wait(sem_used) < 0) continue;

        current = cb_pop(&shm->cb);
        
        if(current.count == 0) {
            shm->quit = 1;
            quit = 1;
            fprintf(stdout, "The graph is acyclic!\n");
        }
        else if(current.count < smallest.count) {
            smallest = current;
            if(fprintf(stdout, "Solution with %d edges: ", (int) smallest.count) < 0) error_str("Printing to stdout failed");
            if(print_edges(stdout, smallest.edges, smallest.count) < 0) error_str("Printing to stdout failed");
            if(fprintf(stdout, "\n") < 0) error_str("Printing to stdout failed");
        }

        if(sem_post(sem_free) < 0) error_str("Unable to post semaphore");
    }

    shm->quit = 1;


    if (close(shmfd) == -1) error_str("Closing shared memory failed.");

    // unmap shared memory:
    if (munmap(shm, sizeof(*shm)) == -1) error_str("Unmapping shared memory failed");

    // remove shared memory object:
    if (shm_unlink(SHM_NAME) == -1) error_str("Unlinking shared memory failed");

    if(sem_close(sem_free) < 0) error_str("Closing semaphore failed");
    if(sem_close(sem_used) < 0) error_str("Closing semaphore failed");
    
    if(sem_unlink(CB_SEM_FREE) < 0) error_str("Unlinking semaphore failed");
    if(sem_unlink(CB_SEM_USED) < 0) error_str("Unlinking semaphore failed");

    return 0;
}
