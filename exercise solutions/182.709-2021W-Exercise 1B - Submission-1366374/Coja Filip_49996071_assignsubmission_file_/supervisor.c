/**
 * @file supervisor.c
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 11.11.2021
 *
 * @brief Supervisor program which manages and handles each generator program. Searchs for the least amount of edges deleted.
 *
 * @details Sets up signal handling and creates circular buffer as server, so all generator programs can use the shared memory.
 * Same goes for the semaphores. It searches for the solution with the least edges removed.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include "circular_buffer.h"

/** Add signal handling so everything gets closed properly */
volatile sig_atomic_t quit = 0;

void handle_signal(int signal) { quit = 1; }

/**
 * @brief Main entry point for supervisor program.
 * @details Main function.
 *
 * @param argc Argument counter
 * @param argv Argument variable
 * @return exit code
 */
int main(int argc, char **argv) {
    if (argc > 1) {
        fprintf(stderr, "[./supervisor] ERROR: This program doesn't take in any args. \nUsage: ./supervisor \n");
    }

    /** Signal handling */
    struct sigaction sa = {.sa_handler = handle_signal};
    sigaction(SIGINT, &sa, NULL);

    /** Open Circular Buffer */
    circular_buffer_t *cbuff = open_cbuff(true);
    if (cbuff == NULL) {
        fprintf(stderr, "[./supervisor] Error opening Circular Buffer. \n");
        exit(EXIT_FAILURE);
    }

    /** Get new items from circular buffer */
    long min_deletions = -1;
    while (!quit) {
        long size = read_buffer(cbuff);
        if (size == -1) {
            if (errno == EINTR) continue;

            if (!close_cbuff(cbuff, false)) {
                fprintf(stderr, "[./supervisor] ERROR: Couldn't close circular buffer. \n");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "[./supervisor] ERROR: Interrupt error. \nUsage: ./supervisor \n");
            exit(EXIT_FAILURE);
        }
        long deletions = size / 2;

        if (min_deletions == -1 || deletions < min_deletions) {
            min_deletions = deletions;
            if (min_deletions == 0) {
                printf("[./supervisor] The graph is 3-colorable!\n");
                quit = 1;
            } else {
                printf("[./supervisor] Solution with %ld edges:", min_deletions);
                print_solution_string(cbuff, size);
                printf("\n");
            }
        } else {
            skip_solution(cbuff, size);
        }
    }

    /** Stop generator processes and close buffer */
    cbuff->shm->halt = true;
    if (!close_cbuff(cbuff, true)) {
        fprintf(stderr, "[./supervisor] ERROR: Couldnt close circular buffer. \n");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
