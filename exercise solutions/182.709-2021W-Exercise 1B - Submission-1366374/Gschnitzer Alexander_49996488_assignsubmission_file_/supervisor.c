/**
 * @file supervisor.c
 * @author Alexander Gschnitzer (01652750) <e1652750@student.tuwien.ac.at>
 * @date 21.10.2021
 *
 * @brief Displays the best solution of 3-coloring problem found by generator.c.
 * @details Sets up shared memory, the semaphores and circular buffer to wait for solutions proposed by the generators.
 * Prints out the solution with the least edges removed and terminates if the graph is acyclic, i.e. a solution with 0 edges is found.
 */

#include "util.h"
#include <stdio.h>
#include <strings.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

/**
 * @brief Global variables prog_name and arguments - used in util.c to display usage message.
 */
const char *prog_name, *arguments = NULL;

/**
 * @brief Shared memory flags to define access of file - used in util.c. 0_RDWR means read and write access,
 * 0_CREAT creates the file if it does not exist and 0_EXCL returns an error if it already exists, i.e. was not properly removed.
 */
const int shm_flags = O_RDWR | O_CREAT | O_EXCL, sem_flags = O_CREAT | O_EXCL;

/**
 * @brief Circular buffer acting as the mapped shared memory object.
 */
cb_t *buffer;

/**
 * @brief Semaphores that store information about free and used space in circular buffer respectively.
 * sem_access is used to prevent concurrent access to circular buffer.
 */
sem_t *sem_free, *sem_used, *sem_access;

/**
 * @brief Async-safe variable that is set to terminate program if signal interrupts execution.
 */
static volatile sig_atomic_t quit = 0;

/**
 * Signal handler.
 * @brief Handles signals that terminate the program.
 * @details Sets global async-safe variable 'quit' to true which will terminate the program.
 * @param _ signal that interrupted execution - required by sigaction handler.
 */
static void handle_signal(int _) {
    quit = 1;
}

/**
 * Main entry point of supervisor
 * @brief Prints out solutions to the 3-coloring problem.
 * @details Configures signal handler, initializes shared memory object and semaphores - calls function from util.c.
 * Reads from the buffer until solution with 0 removed edges is found or the execution is interrupted by a signal.
 * Prints the best solution to stdout and uses two semaphores to regulate free and used space respectively.
 * @param argc Number of command-line parameters in argv.
 * @param argv Array of command-line parameters, argc elements long.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char **argv) {
    // init signal handler
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // set program name
    prog_name = argv[0];

    int c;
    while ((c = getopt(argc, argv, "\0")) != -1) {
        if (c == '?') {
            usage();
        }
    }

    // check number of positional arguments
    if ((argc - optind) >= 1) {
        usage();
    }

    // create shared memory object
    init_buffer(1);

    // initialize free-space, used-space and access semaphores
    init_sem(1);

    // initialize the best solution with the highest number of removed edges possible
    solution_t *best = malloc(sizeof(solution_t));
    memset(best, MAX_EDGES, sizeof(solution_t));

    while (!quit) {
        // blocks until circular buffer is free again
        if (sem_wait(sem_used) == -1 && errno != EINTR) {
            error("Reading solution from queue failed");
        }

        // read solution entry from queue
        solution_t solution = buffer->queue[buffer->r_pos];

        // increment read position while ensuring that it stays within the bounds of the queue
        buffer->r_pos += 1;
        buffer->r_pos %= MAX_SIZE;

        // increments semaphore for free space in buffer
        if (sem_post(sem_free) == -1 && errno != EINTR) {
            error("Reading solution from queue failed");
        }

        if (solution.removed < best->removed) {
            // prevent output of solution if process is interrupted by signal
            if (!quit) {
                *best = solution;
                print_solution(&solution);
            }

            // if graph is acyclic, i.e. solution with 0 removed edges is found, the supervisor and generators terminate
            if (solution.removed == 0) {
                buffer->terminate = 1;
                break;
            }
        }
    }

    // notify generators to terminate
    buffer->terminate = 1;

    // clean up allocated memory
    clear_buffer(1);
    clear_sem(1);

    exit(EXIT_SUCCESS);
}
