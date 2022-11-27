/**
 * @file generator.c
 * @author Jiang Yuchen 12019845
 * @date 31.10.2021
 * @brief Generator program
 **/

#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <regex.h>
#include <time.h>
#include "main.h"

volatile sig_atomic_t quit = 0;

/**
 * Handles the signals SIGINT and SIGTERM
 * @brief Sets the quit variable to 1, so the loop terminates
 * @param signal signal
 */
void handle_signal(int signal) {
    quit = 1;
}

/**
 * Main method
 * @brief This method contains all the functionalities of the generator.
 * It takes the given graph and tries to find a feedback arc set and puts it into the shared memory
 * @param argc Stores the number of arguments
 * @param argv Array of positional arguments in format ^([0-9]+)-([0-9]+)$
 * @return EXIT_SUCCESS
 */
int main(int argc, char** argv) {
    // Signals
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Arguments
    long max_node = 0;
    long graph_from[argc-1];
    long graph_to[argc-1];
    if (argc > 1) {
        char *rest;
        char *empty;
        long temp_num;
        regex_t regex;
        int r = regcomp(&regex, "^([0-9]+)-([0-9]+)$", REG_EXTENDED);
        if (r) {
            printf("Regex compile failed\n");
            exit(EXIT_FAILURE);
        }
        for (int i = 1; i < argc; ++i) {
            r = regexec(&regex, argv[i], 0, NULL, 0);
            if (r == 0) {
                temp_num = strtol(argv[i], &rest, 10);
                if (temp_num > max_node) max_node = temp_num;
                graph_from[i-1] = temp_num;
                rest += 1;
                temp_num = strtol(rest, &empty, 10);
                if (temp_num > max_node) max_node = temp_num;
                graph_to[i-1] = temp_num;
            } else {
                fprintf(stderr, "[%s] Wrong argument \"%s\" (Usage: generator EDGE1 [EDGE2 ...])\n", argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        regfree(&regex);
    } else {
        fprintf(stderr, "[%s] No arguments (Usage: generator EDGE1 [EDGE2 ...])\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create vertices array
    long nodes[max_node + 1];
    for (int i = 0; i < max_node + 1; ++i) {
        nodes[i] = i;
    }

    // Shared memory
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "[%s] Error shared memory file descriptor: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Map shared memory
    struct shm *buffer;
    buffer = mmap(NULL, sizeof (*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buffer == MAP_FAILED) {
        fprintf(stderr, "[%s] Error memory mapping: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Semaphores
    sem_t *freesem = sem_open("/12019845freesem", O_CREAT);
    sem_t *usedsem = sem_open("/12019845usedsem", O_CREAT);
    sem_t *mutexsem = sem_open("/12019845mutexsem", O_CREAT);
    if (freesem == SEM_FAILED || usedsem == SEM_FAILED || mutexsem == SEM_FAILED) {
        fprintf(stderr, "[%s] Error semaphore: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Variable for solutions
    struct edges fbarc;
    memset(&fbarc, 0, sizeof fbarc);

    long random;
    long temp;
    long node;
    long fbarc_i = 0;
    int too_large = 0;

    // Calculates possible solutions and puts them into the shared memory
    while(!quit && buffer->stop == 0) {
        // Shuffle
        for (long i = max_node; i > 0 ; --i) {
            random = (rand() % (i+1));
            if (random == i) continue;
            temp = nodes[random];
            nodes[random] = nodes[i];
            nodes[i] = temp;
        }

        // Search for feedback arcs
        for (int i = 0; i < max_node+1 && !too_large; ++i) { /* Goes through each node */
            node = nodes[i];
            for (int j = 0; j < argc-1 && !too_large; ++j) { /* Goes through all the edges */
                if (graph_from[j] == node) {
                    for (int k = 0; k <= i; ++k) { /* Goes through each node until i */
                        if (graph_to[j] == nodes[k]) { /* Check if there are edges going backwards and adds them to the solution*/
                            fbarc.from[fbarc_i] = node;
                            fbarc.to[fbarc_i] = graph_to[j];
                            fbarc_i++;
                            if (fbarc_i >= FBARC_MAX - 1) too_large = 1;  /* discards solutions that are too large */
                            break;
                        }
                    }
                }
            }
        }

        // Puts valid solutions into the shared Memory
        if (!too_large) {
            struct timespec after_2_sec = {.tv_sec = time(NULL) + 2, .tv_nsec = 0};
            errno = 0;
            sem_timedwait(freesem, &after_2_sec);
            if (errno == ETIMEDOUT) continue;
            if (errno == EINTR) break;
            errno = 0;
            sem_wait(mutexsem);
            if (errno == EINTR) break;
            buffer->edges[buffer->wr_pos] = fbarc;
            buffer->edges_size[buffer->wr_pos] = fbarc_i;
            buffer->wr_pos += 1;
            buffer->wr_pos %= MAX_DATA;
            sem_post(mutexsem);
            sem_post(usedsem);
            fbarc_i = 0;
            memset(&fbarc, 0, sizeof fbarc);
        } else {
            too_large = 0;
            fbarc_i = 0;
            memset(&fbarc, 0, sizeof fbarc);
        }
    }

    // Close resources
    if (close(shmfd) == -1) {
        fprintf(stderr, "[%s] Error close shmfd: %s", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    sem_close(freesem);
    sem_close(usedsem);
    sem_close(mutexsem);
    return EXIT_SUCCESS;
}

