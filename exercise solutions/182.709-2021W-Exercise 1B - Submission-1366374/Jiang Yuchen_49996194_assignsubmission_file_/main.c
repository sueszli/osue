/**
 * @file main.c
 * @author Jiang Yuchen 12019845
 * @date 31.10.2021
 * @brief Supervisor program
 **/

#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include "main.h"

volatile sig_atomic_t quit = 0;

/**
 * Handles the signals SIGINT and SIGTERM
 * @brief Sets the quit variable to 1, so the loop terminates.
 * @param signal signal
 */
void handle_signal(int signal) {
    quit = 1;
}

/**
 * Main method
 * @brief This method contains all the functionalities of the supervisor.
 * It takes the feedback arc set from shared memory and prints it, if it is the smallest one found until the given moment.
 * @param argc Stores the number of arguments
 * @param argv No arguments accepted
 * @return EXIT_SUCCESS
 */
int main(int argc, char** argv) {
    // Arguments
    if (argc > 1) {
        fprintf(stderr, "[%s] Program takes no arguments (Usage: supervisor)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Signals
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Shared memory
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "[%s] Error shared memory file descriptor: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Resize shared memory
    if (ftruncate(shmfd, sizeof(struct shm)) < 0) {
        fprintf(stderr, "[%s] Error ftruncate: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Map shared memory
    struct shm *buffer;
    buffer = mmap(NULL, sizeof (*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buffer == MAP_FAILED) {
        fprintf(stderr, "[%s] Error mmap: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Initialise buffer
    buffer->wr_pos = 0;
    buffer->stop = 0;

    // Semaphores
    sem_t *freesem = sem_open("/12019845freesem", O_CREAT, 0600, 10);
    sem_t *usedsem = sem_open("/12019845usedsem", O_CREAT, 0600, 0);
    sem_t *mutexsem = sem_open("/12019845mutexsem", O_CREAT, 0600, 1);
    if (freesem == SEM_FAILED || usedsem == SEM_FAILED || mutexsem == SEM_FAILED) {
        fprintf(stderr, "[%s] Error semaphore: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    int rd_pos = 0;
    struct edges fbarc;
    int min_edges = INT_MAX;

    // Reads solutions and prints the best ones
    while(!quit) {
        errno = 0;
        sem_wait(usedsem);
        if (errno == EINTR) break;
        if (buffer->edges_size[rd_pos] < min_edges) {
            min_edges = buffer->edges_size[rd_pos];
            fbarc = buffer->edges[rd_pos];
            if (min_edges > 0) {
                printf("[%s] Solution with %d edges: ", argv[0], min_edges);
                for (int i = 0; i < min_edges; ++i) {
                    printf("%ld-%ld ", fbarc.from[i], fbarc.to[i]);
                }
                printf("\n");
            } else {
                printf("[%s] The graph is acyclic!\n", argv[0]);
            }
        }
        sem_post(freesem);
        rd_pos++;
        rd_pos %= MAX_DATA;
        if (min_edges == 0) break;
    }

    // Stop generators
    buffer->stop = 1;

    // Close resources
    if (munmap(buffer, sizeof (*buffer)) == -1) {
        fprintf(stderr, "[%s] Error memory unmap: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(shmfd) == -1) {
        fprintf(stderr, "[%s] Error close shmfd: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (shm_unlink(SHM_NAME) == -1) {
        fprintf(stderr, "[%s] Error unlink shm: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    sem_close(freesem);
    sem_close(usedsem);
    sem_close(mutexsem);
    sem_unlink("/12019845freesem");
    sem_unlink("/12019845usedsem");
    sem_unlink("/12019845mutexsem");
    return EXIT_SUCCESS;
}
