/**
 * @file generator.c
 * @author Valentin Futterer 11904654
 * @date 05.11.2021
 * @brief Writes solutions for three-coloring problem to circular buffer.
 * @details This Programm takes no options and at least one Edge as input. It opens the same shared memory circular
 * buffer as supervisor and the same semaphores. Then it repeatedly generates random solutions to the input graph
 * and writes them into the circular buffer. If the quit flag in the circualr buffer is 1, the program terminates.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include "3-coloringUtil.h"

/**
 * @brief Displays usage message.
 * @details Prints a message to stderr and shows the correct way to parse arguments to the program, exits with error.
 * @param prog_name Programme name to print in error message.
 * @return Returns nothing.
*/
static void usage(const char *prog_name) {
    fprintf(stderr,"Usage: %s  EDGE1...\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Randomly colors graph.
 * @details Uses rand to assign each Node in the nodes array a random Color,which is a enum.
 * @param nodes The nodes to color.
 * @param size The size of the nodes array.
 * @return Returns nothing.
*/
static void rand_color_graph(Colors nodes[], int size) {
    for (size_t i = 0; i < size; i++) {
        nodes[i] = rand() % 3;
    }
}

// global quit flag
volatile sig_atomic_t quit = 0;
/**
 * @brief Handles a signal.
 * @details Sets global quit flag to 1 if the signal is received.
 * @param signal The received signal.
 * @return Returns nothing.
*/
void handle_signal(int signal) {
    quit = 1;
}

/**
 * @brief Main program body.
 * @details At first checks if at least one Edge was given as positional argument. Then it opens the
 * circular buffer and semaphores already openend by supervisor. The rand_color_graph method is used to generate a
 * random three-coloring and all the edges that contradict the solution are removed. If there are less than 8 edges removed
 * the solution is written to the circular buffer. Then a new solution gets generated. The program terminates if the quit
 * Flag in the circular buffer is 1, or the signals SIGINT or SIGTERM are received.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns 0 on success.
 */
int main(int argc, char* const argv[])
{
    //set a random seed using time
    srand(time(NULL));

    //signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //check if there are no options
    if (getopt(argc, argv, "") != -1) {
        usage(argv[0]);
    }
    // check if there is at least one argument
    if ((argc - optind) == 0) {
        usage(argv[0]);
    }

    // open shm and semaphores
    Circ_buf *circ_buf = open_circular_buffer(argv[0]);

    sem_t *sem_read = sem_open(SEM_READ, 0);
    if (sem_read == SEM_FAILED) {
        munmap(circ_buf, sizeof(*circ_buf));
        handle_error(argv[0], "Opening of read semaphore has failed");
    }
    
    sem_t *sem_write = sem_open(SEM_WRITE, 0);
    if (sem_write == SEM_FAILED) {
        munmap(circ_buf, sizeof(*circ_buf));
        sem_close(sem_read);
        handle_error(argv[0], "Opening of write semaphore has failed");
    }

    sem_t *sem_mut_excl = sem_open(SEM_MUT_EXCL, 0);
    if (sem_mut_excl == SEM_FAILED) {
        munmap(circ_buf, sizeof(*circ_buf));
        sem_close(sem_read);
        sem_close(sem_write);
        handle_error(argv[0], "Opening of mutual exclusive access semaphore has failed");
    }

    int input_size = argc - optind;
    int node_count = 0;
    Edge *edges = NULL;

    edges = calloc(input_size, sizeof(Edge));
    if (edges == NULL) {
        munmap(circ_buf, sizeof(*circ_buf));
        sem_close(sem_read);
        sem_close(sem_write);
        sem_close(sem_mut_excl);
        handle_error(argv[0], "Assigning memory for input Edges failed");
    }
    
    // reads Edges from positional Arguments
    for (size_t i = 0; i < input_size; i++) {
        //sscanf needs unsigned int * for %u to work
        int sscanf_return = sscanf(argv[optind++], "%u-%u", &edges[i].start, &edges[i].end);
        // error encountered
        if(sscanf_return != 2) {
            free(edges);
            munmap(circ_buf, sizeof(*circ_buf));
            sem_close(sem_read);
            sem_close(sem_write);
            sem_close(sem_mut_excl);
            // some error
            if (sscanf_return == EOF) {
                handle_error(argv[0], "Passing edges from positional arguments failed");
            // wrong format for edge
            } else {
                usage(argv[0]);
            }
        }

        // find out the node count
        if (edges[i].start > node_count) {
            node_count = edges[i].start;
        }
        if (edges[i].end > node_count) {
            node_count = edges[i].end;
        }
    }
    // nodes start with 0, therefor +1
    Colors node_colors[++node_count];
    
    int solution_size = 0;
    Removed_edges solution;

    // loop finding random solutions and writing them
    while (quit != 1) {
        // check the quit flag, is only read, no synchronisation needed
        quit = circ_buf->quit_flag;

        // reset solution size
        solution_size = 0;

        rand_color_graph(node_colors, node_count);
        //generate a solution
        for (size_t i = 0; i < input_size; i++) {
            if (node_colors[edges[i].start] == node_colors[edges[i].end]) {
                if (solution_size < MAX_EDGES_IN_SOLUTION) {
                    solution.removed_edges[solution_size].start = edges[i].start;
                    solution.removed_edges[solution_size].end = edges[i].end;
                    solution_size++;
                } else {
                    // if solution size is already 8, it gets incremented and caught in the next if
                    solution_size++;
                    break;
                }
            }
        }

        //solution is to big, start over
        if (solution_size > MAX_EDGES_IN_SOLUTION) {
            continue;
        }

        if (sem_wait(sem_mut_excl) == -1) {
            if (errno == EINTR) {
                continue;
            }
            free(edges);
            munmap(circ_buf, sizeof(*circ_buf));
            sem_close(sem_read);
            sem_close(sem_write);
            sem_close(sem_mut_excl);
            handle_error(argv[0], "Decreasing mutual exclusive access semaphore failed");
        }

        if (sem_wait(sem_write) == -1) {
            if (errno == EINTR) {
                continue;
            }
            free(edges);
            munmap(circ_buf, sizeof(*circ_buf));
            sem_close(sem_read);
            sem_close(sem_write);
            sem_close(sem_mut_excl);
            handle_error(argv[0], "Decreasing write semaphore failed");
        }

        // write the solution
        unsigned int write_pos = circ_buf->write_pos;
        circ_buf->solution_size[write_pos] = solution_size;
        circ_buf->solution[write_pos] = solution;
        write_pos++;
        write_pos %= sizeof(circ_buf->solution)/sizeof(Removed_edges);
        circ_buf->write_pos = write_pos;

        if (sem_post(sem_read) == -1) {
            if (errno == EINTR) {
                continue;
            }
            free(edges);
            munmap(circ_buf, sizeof(*circ_buf));
            sem_close(sem_read);
            sem_close(sem_write);
            sem_close(sem_mut_excl);
            handle_error(argv[0], "Increasing read semaphore failed");
        }
        
        if (sem_post(sem_mut_excl) == -1) {
            if (errno == EINTR) {
                continue;
            }
            free(edges);
            munmap(circ_buf, sizeof(*circ_buf));
            sem_close(sem_read);
            sem_close(sem_write);
            sem_close(sem_mut_excl);
            handle_error(argv[0], "Increasing mutual exclusive access semaphore failed");
        }
    }

    // Increment write semaphor to enable other generators to read the quit flag in circular buffer
    if (sem_post(sem_write) == -1) {
            if (errno != EINTR) {
                munmap(circ_buf, sizeof(*circ_buf));
                shm_unlink(SHM_NAME);
                sem_unlink(SEM_READ);
                sem_unlink(SEM_WRITE);
                sem_unlink(SEM_MUT_EXCL);
                sem_close(sem_read);
                sem_close(sem_write);
                sem_close(sem_mut_excl);
                handle_error(argv[0], "Increasing writing semaphore failed");
            }
    }

    // cleanup
    free(edges);
    if(munmap(circ_buf, sizeof(*circ_buf)) == -1) {
        handle_error(argv[0], "Unmapping of circular buffer failed");
    }
    if(sem_close(sem_read) == -1) {
        handle_error(argv[0], "Closing of read semaphore failed");
    }
    if(sem_close(sem_write) == -1) {
        handle_error(argv[0], "Closing of writing semaphore failed");
    }
    if(sem_close(sem_mut_excl) == -1) {
        handle_error(argv[0], "Closing of mutual exclusive acess semaphore failed");
    }
    return 0;
}
