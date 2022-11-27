/**
 * @file supervisor.c
 * @author Valentin Futterer 11904654
 * @date 05.11.2021
 * @brief Reads solutions for three-coloring problem from circular buffer, prints new best solutions.
 * @details This Programm takes no arguments and options. It opens a shared memeory circular buffer, a write, read
 * and a mutual excluxive access semaphore for the curcular buffer. Then it waits for solutions written to circular
 * buffer. If a read solution is better than the old one, it is printed to stdout. If a solution with 0 edges removed is
 * read or it receives the signals SIGINT or SIGTERM, the quit flag in the buffer is set. Then it terminates.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include "3-coloringUtil.h"

/**
 * @brief Displays usage message.
 * @details Prints a message to stderr and shows the correct way to parse arguments to the program, exits with error.
 * @param prog_name Programme name to print in error message.
 * @return Returns nothing.
*/
static void usage(const char *prog_name) {
    fprintf(stderr,"Usage: %s\n", prog_name);
    exit(EXIT_FAILURE);
}

//global quit flag
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
 * @brief Prints a solution to stdout.
 * @details Loops through a solution using solution_size, to only read relevant parts.
 * @param best_solution The solution to print.
 * @param solution_size How many array fields are relevant.
 * @return Returns -1 on error, 0 on success.
*/
static int print_solution(Removed_edges best_solution, int solution_size) {
    // reads only relevant parts out of the solution array
    for (size_t i = 0; i < solution_size; i++) {
        if (fprintf(stdout, " %d-%d", best_solution.removed_edges[i].start, best_solution.removed_edges[i].end) < 0) {
            return -1;
        }
    }
    if (fprintf(stdout, "\n") < 0) {
        return -1;
    }
    return 0;
}

/**
 * @brief Main program body.
 * @details At first checks if no arguments and options are given. Then opens the circular buffer and read, write, 
 * mutual exclusive access semaphores. The prgram then starts reading solutions from the circular buffer. If a solution
 * that is better than the currently best is read, it is printed to stdout. If a solution with 0 edges is found, the
 * graph is acyclic and it is written to stdout. In this case, the quit flag in circualr buffer is set and the programm
 * exits. The same happens when the signals SIGING or SIGTERM are caught.
 * @param argc The argument counter. (should be 1)
 * @param argv The argument vector. (should only be the programe name)
 * @return Returns 0 on success.
 */
int main(int argc, char* const argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // no options and positial arguments allowed
    if (getopt(argc, argv, "") != -1) {
        usage(argv[0]);
    }
    if ((argc - optind) != 0) {
        usage(argv[0]);
    }
    
    // set quit flag and write_pos for generators
    Circ_buf *circ_buf = open_circular_buffer(argv[0]);
    circ_buf-> quit_flag = 0;
    circ_buf-> write_pos = 0;

    sem_t *sem_read = sem_open(SEM_READ, O_CREAT | O_EXCL, 0600, 0);
    if (sem_read == SEM_FAILED) {
        munmap(circ_buf, sizeof(*circ_buf));
        shm_unlink(SHM_NAME);
        handle_error(argv[0], "Opening of read semaphore has failed");
    }
    
    sem_t *sem_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, sizeof(circ_buf->solution)/sizeof(Removed_edges));
    if (sem_write == SEM_FAILED) {
        munmap(circ_buf, sizeof(*circ_buf));
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_READ);
        sem_close(sem_read);
        handle_error(argv[0], "Opening of write semaphore has failed");
    }

    sem_t *sem_mut_excl = sem_open(SEM_MUT_EXCL, O_CREAT | O_EXCL, 0600, 1);
    if (sem_mut_excl == SEM_FAILED) {
        munmap(circ_buf, sizeof(*circ_buf));
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_READ);
        sem_unlink(SEM_WRITE);
        sem_close(sem_read);
        sem_close(sem_write);
        handle_error(argv[0], "Opening of mutual exclusive access semaphore has failed");
    }

    int read_pos = 0;
    int best_solution_size = MAX_EDGES_IN_SOLUTION + 1;
    //stores current solution
    Removed_edges removed_edges;
    //loops as long as the graph is not acyclic and the quit flag is not set
    while (quit != 1 && best_solution_size != 0) {
        if (sem_wait(sem_read) == -1) {
            if (errno == EINTR) {
                continue;
            }
            munmap(circ_buf, sizeof(*circ_buf));
            shm_unlink(SHM_NAME);
            sem_unlink(SEM_READ);
            sem_unlink(SEM_WRITE);
            sem_unlink(SEM_MUT_EXCL);
            sem_close(sem_read);
            sem_close(sem_write);
            sem_close(sem_mut_excl);
            handle_error(argv[0], "Decreasing read semaphore failed");
        }
        // read solution
        removed_edges = circ_buf->solution[read_pos];
        // check if new solution is better than old one
        // if yes, print it and check if the graph is acyclic
        if (circ_buf->solution_size[read_pos] < best_solution_size) {
            best_solution_size = circ_buf->solution_size[read_pos];
            
            if (fprintf(stdout, "New solution with %d edges removed found", best_solution_size) < 0) {
                munmap(circ_buf, sizeof(*circ_buf));
                shm_unlink(SHM_NAME);
                sem_unlink(SEM_READ);
                sem_unlink(SEM_WRITE);
                sem_unlink(SEM_MUT_EXCL);
                sem_close(sem_read);
                sem_close(sem_write);
                sem_close(sem_mut_excl);
                handle_error(argv[0], "Printing new solution message failed");
            }
            if(print_solution(removed_edges, best_solution_size) < 0) {
                munmap(circ_buf, sizeof(*circ_buf));
                shm_unlink(SHM_NAME);
                sem_unlink(SEM_READ);
                sem_unlink(SEM_WRITE);
                sem_unlink(SEM_MUT_EXCL);
                sem_close(sem_read);
                sem_close(sem_write);
                sem_close(sem_mut_excl);
                handle_error(argv[0], "Printing new solution failed");
            }
            if(fflush(stdout) == EOF) {
                munmap(circ_buf, sizeof(*circ_buf));
                shm_unlink(SHM_NAME);
                sem_unlink(SEM_READ);
                sem_unlink(SEM_WRITE);
                sem_unlink(SEM_MUT_EXCL);
                sem_close(sem_read);
                sem_close(sem_write);
                sem_close(sem_mut_excl);
                handle_error(argv[0], "Flushing stdout failed");
            }
        }

        if (best_solution_size == 0) {
            if (fprintf(stdout, "The graph is acyclic\n") < 0) {
                munmap(circ_buf, sizeof(*circ_buf));
                shm_unlink(SHM_NAME);
                sem_unlink(SEM_READ);
                sem_unlink(SEM_WRITE);
                sem_unlink(SEM_MUT_EXCL);
                sem_close(sem_read);
                sem_close(sem_write);
                sem_close(sem_mut_excl);
                handle_error(argv[0], "Printing acyclic message failed");
            }
            if(fflush(stdout) == EOF) {
                munmap(circ_buf, sizeof(*circ_buf));
                shm_unlink(SHM_NAME);
                sem_unlink(SEM_READ);
                sem_unlink(SEM_WRITE);
                sem_unlink(SEM_MUT_EXCL);
                sem_close(sem_read);
                sem_close(sem_write);
                sem_close(sem_mut_excl);
                handle_error(argv[0], "Flushing stdout failed");
            }
        }
        if (sem_post(sem_write) == -1) {
            if (errno == EINTR) {
                continue;
            }
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
        //set read_pos for next iteration
        read_pos += 1;
        read_pos %= sizeof(circ_buf->solution)/sizeof(Removed_edges);
    }

    // After leaving the loop, set the quit flag, so that the generators can quit
    circ_buf->quit_flag = 1;

    // This post enables at least one generator to read the quit flag
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

    //cleanup
    if(munmap(circ_buf, sizeof(*circ_buf)) == -1) {
        handle_error(argv[0], "Unmapping of circular buffer failed");
    }
    if(shm_unlink(SHM_NAME) == -1) {
        handle_error(argv[0], "Unlinking of circular buffer failed");
    }
    if(sem_unlink(SEM_READ) == -1) {
        handle_error(argv[0], "Unlinking of read semaphore failed");
    }
    if(sem_close(sem_read) == -1) {
        handle_error(argv[0], "Closing of read semaphore failed");
    }
    if(sem_unlink(SEM_WRITE) == -1) {
        handle_error(argv[0], "Unlinking of read semaphore failed");
    }
    if(sem_close(sem_write) == -1) {
        handle_error(argv[0], "Closing of writing semaphore failed");
    }
    if(sem_unlink(SEM_MUT_EXCL) == -1) {
        handle_error(argv[0], "Unlinking of read semaphore failed");
    }
    if(sem_close(sem_mut_excl) == -1) {
        handle_error(argv[0], "Closing of mutual exclusive acess semaphore failed");
    }
    return 0;
}
