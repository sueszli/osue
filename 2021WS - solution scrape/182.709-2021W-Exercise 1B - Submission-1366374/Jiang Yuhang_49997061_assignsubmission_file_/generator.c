/**
 * @file generator.c
 * @author Yuhang Jiang <e12019854@student.tuwien.ac.at>
 * @brief Start point of the generator program
 * @details The generator program generates random solutions and writes then to the circular
 * buffer.
 * @date 30.10.2021
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <regex.h>
#include "circular_buffer.h"

#define SHM_NAME "/12019854_3color"

/**
 * @brief Program name
 */
char *PROG_NAME;

/**
 * @brief Struct for the shared memory
 */
struct shm_t {
    int state;
    struct circular_buffer cbuf;
};

/**
 * @brief Flag to tell the program to end
 */
volatile sig_atomic_t quit = 0;

/**
 * @brief Handler for SIGINT and SIGTERM
 * @details Sets the quit flag to 1 to signal the program to quit
 * @param signal
 */
void handle_signal(int signal) {
    quit = 1;
}

/**
 * @brief Main function of the supervisor.
 * @details First the signal handler is set up so that SIGINT or SIGTERM will attempt to close all
 * resources and end the program. Then the input is checked and parsed. After that, a random solution
 * is generated and it's length is checked. If the solution is less than MAX_EDGES long, it is written
 * into the circular buffer.
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS if the program finishes as expected. EXIT_FAILURE if the input is invalid
 * or if something fails.
 */
int main(int argc, char *argv[])
{
    PROG_NAME = argv[0];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_signal;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s EDGE1 [EDGE2 ...]\n", PROG_NAME);
        exit(EXIT_FAILURE);
    }

    const unsigned int nr_of_edges = argc-1;
    int edge_from[nr_of_edges];
    int edge_to[nr_of_edges];
    int nr_of_nodes = 0;

    regex_t regex;
    if (regcomp(&regex, "^([0-9]+)-([0-9]+)$", REG_EXTENDED) != 0) {
        fprintf(stderr, "[%s] ERROR: regcomp failed: %s", PROG_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (regexec(&regex, argv[i], 0, NULL, 0) == REG_NOMATCH) {
            fprintf(stderr,"[%s] Invalid input: %s\nUsage: %s EDGE1 [EDGE2 ...]\n", PROG_NAME, argv[i], PROG_NAME);
            exit(EXIT_FAILURE);
        }
        char *endptr;
        edge_from[i - 1] = (int) strtol(argv[i], &endptr, 10);
        if (edge_from[i - 1] > nr_of_nodes) {
            nr_of_nodes = edge_from[i - 1];
        }
        endptr++;
        edge_to[i - 1] = (int) strtol(endptr, NULL, 10);
        if (edge_to[i - 1] > nr_of_nodes) {
            nr_of_nodes = edge_to[i - 1];
        }
    }

    nr_of_nodes++;  // 0 is also a Node

    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "[%s] ERROR: shm_open failed: %s\n"
                        "[%s] Likely the supervisor has not started yet.\n", PROG_NAME, strerror(errno), PROG_NAME);
        exit(EXIT_FAILURE);
    }

    struct shm_t *graph_buffer;
    graph_buffer = mmap(NULL, sizeof(*graph_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (graph_buffer == MAP_FAILED) {
        fprintf(stderr, "[%s] ERROR: mmap failed: %s", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(shmfd) == -1) {
        fprintf(stderr, "[%s] ERROR: close failed: %s", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    sem_t *free_sem = sem_open("/12019854_free_sem", 0, 0600);
    sem_t *used_sem = sem_open("/12019854_used_sem", 0, 0600);
    sem_t *write_sem = sem_open("/12019854_write_sem", 0, 0600);

    if (free_sem == SEM_FAILED || used_sem == SEM_FAILED || write_sem == SEM_FAILED) {
        fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", PROG_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (graph_buffer->state == 1 && quit != 1) {
        int colors[nr_of_nodes];
        for (int i = 0; i < nr_of_nodes; ++i) {
            int rand_color = rand() % 3;
            colors[i] = rand_color;
        }

        struct edges sel_edges = {.solution_len = 0, .colorable = 0};

        for (int i = 0; i < nr_of_edges; ++i) {
            if (colors[edge_from[i]] != colors[edge_to[i]]) {
                if (sel_edges.solution_len > MAX_EDGES) {
                    break;
                }
                sel_edges.node1[sel_edges.solution_len] = edge_from[i];
                sel_edges.node2[sel_edges.solution_len] = edge_to[i];
                sel_edges.solution_len++;
            }
        }

        if (sel_edges.solution_len == nr_of_edges) {
            sel_edges.colorable = 1;
        }

        if (sel_edges.solution_len <= MAX_EDGES) {
            struct timespec wait_time = {.tv_sec = time(NULL) + 2, .tv_nsec = 0};
            errno = 0;
            sem_timedwait(free_sem, &wait_time);
            if (errno == ETIMEDOUT) {
                continue;
            } else if (errno == EINTR) {
                break;
            }
            sem_wait(write_sem);
            if (errno == EINTR) break;
            cbuf_write(&graph_buffer->cbuf, &sel_edges);
            sem_post(write_sem);
            sem_post(used_sem);
        }
    }

    if (munmap(graph_buffer, sizeof(*graph_buffer)) == -1) {
        fprintf(stderr, "[%s] ERROR: munmap failed: %s", PROG_NAME, strerror(errno));
    }

    if (sem_close(write_sem) != 0) {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s", PROG_NAME, strerror(errno));
    }
    if (sem_close(free_sem) != 0) {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s", PROG_NAME, strerror(errno));
    }
    if (sem_close(used_sem) != 0) {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s", PROG_NAME, strerror(errno));
    }

    return EXIT_SUCCESS;
}
