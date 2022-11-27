/**
 * @file supervisor.c
 * @author Yuhang Jiang <e12019854@student.tuwien.ac.at>
 * @brief Start point of the supervisor program
 * @details The supervisor program reads from a shared circular buffer and prints out the solutions.
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
#include <semaphore.h>
#include "circular_buffer.h"

#define SHM_NAME "/12019854_3color"

/**
 * @brief Struct for the shared memory
 */
struct shm_t {
    int state;      /**< state == 0 means the buffer is closed */
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
 * @brief The program name
 */
char *PROG_NAME;

/**
 * @brief Main function of the supervisor. Reads solutions from a circular buffer and prints them.
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS if the program finishes as expected. EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[])
{
    PROG_NAME = argv[0];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_signal;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "[%s] ERROR: shm_open failed: %s", PROG_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmfd, sizeof(struct shm_t)) < 0) {
        fprintf(stderr, "[%s] ERROR: ftruncate failed: %s", PROG_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct shm_t *graph_buffer;
    graph_buffer = mmap(NULL, sizeof(*graph_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (graph_buffer == MAP_FAILED) {
        fprintf(stderr, "[%s] ERROR: mmap failed: %s", PROG_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    graph_buffer->state = 1;

    if (close(shmfd) == -1) {
        fprintf(stderr, "[%s] ERROR: close failed: %s", PROG_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    sem_t *free_sem = sem_open("/12019854_free_sem", O_CREAT | O_EXCL, 0600, CIRCULAR_BUFFER_SIZE);
    sem_t *used_sem = sem_open("/12019854_used_sem", O_CREAT | O_EXCL, 0600, 0);
    sem_t *write_sem = sem_open("/12019854_write_sem", O_CREAT | O_EXCL, 0600, 1);

    if (free_sem == SEM_FAILED || used_sem == SEM_FAILED || write_sem == SEM_FAILED) {
        fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", PROG_NAME, strerror(errno));
        return -1;
    }

    unsigned int best_solution = 0;
    while (quit != 1) {
        errno = 0;
        sem_wait(used_sem);
        if (errno == EINTR) break;
        struct edges read_edges = cbuf_read(&graph_buffer->cbuf);
        sem_post(free_sem);
        if (read_edges.colorable == 1) {
            printf("[%s] The graph is 3-colorable!\n", PROG_NAME);
            quit = 1;
        } else if (read_edges.solution_len > best_solution) {
            best_solution = read_edges.solution_len;
            printf("[%s] Solution with %d edges: ", PROG_NAME, read_edges.solution_len);
            for (int i = 0; i < read_edges.solution_len; ++i) {
                printf("%d-%d ", read_edges.node1[i], read_edges.node2[i]);
            }
            printf("\n");
        }
    }

    graph_buffer->state = 0;

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
    if (sem_unlink("/12019854_free_sem") != 0) {
        fprintf(stderr, "[%s] ERROR: sem_unlink failed: %s", PROG_NAME, strerror(errno));
    }
    if (sem_unlink("/12019854_used_sem") != 0) {
        fprintf(stderr, "[%s] ERROR: sem_unlink failed: %s", PROG_NAME, strerror(errno));
    }
    if (sem_unlink("/12019854_write_sem") != 0) {
        fprintf(stderr, "[%s] ERROR: sem_unlink failed: %s", PROG_NAME, strerror(errno));
    }

    if (shm_unlink(SHM_NAME) != 0) {
        fprintf(stderr, "[%s] ERROR: shm_unlink failed: %s", PROG_NAME, strerror(errno));
    }
    return EXIT_SUCCESS;
}
