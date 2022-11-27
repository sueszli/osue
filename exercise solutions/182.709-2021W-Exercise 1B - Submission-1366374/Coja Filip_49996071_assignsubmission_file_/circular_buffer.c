/**
 * @file circular_buffer.c
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 11.11.2021
 */

#include <stdbool.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "circular_buffer.h"
#include "shm.h"

#define SEM_FREE "12023141_free"
#define SEM_USED "12023141_used"
#define SEM_MUTEX "12023141_mutex"

circular_buffer_t *open_cbuff(bool server) {
    /** Set up shared memory */
    shared_memory_t *shm;
    int fd = -1;
    if ((shm = open_shm(&fd, server)) == NULL) return NULL;

    /** Create buffer and set up semaphores */
    circular_buffer_t *cbuff = malloc(sizeof(circular_buffer_t));
    cbuff->shm = shm;
    cbuff->fd = fd;
    /** Create Semaphores as server, so with access rights and creation rights */
    if (server) {
        if ((cbuff->sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_DATA)) == SEM_FAILED) {
            free(cbuff);
            close_shm(shm, fd, server);
            return NULL;
        }
        if ((cbuff->sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED) {
            sem_close(cbuff->sem_free);
            sem_unlink(SEM_FREE);

            free(cbuff);
            close_shm(shm, fd, server);
            return NULL;
        }

        if ((cbuff->sem_mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1)) == SEM_FAILED) {
            sem_close(cbuff->sem_free);
            sem_unlink(SEM_FREE);
            sem_close(cbuff->sem_free);
            sem_unlink(SEM_FREE);

            free(cbuff);
            close_shm(shm, fd, server);
            return NULL;
        }
    } else {
        /** Create semaphores as clients */
        if ((cbuff->sem_free = sem_open(SEM_FREE, 0)) == SEM_FAILED) {
            free(cbuff);
            return NULL;
        }
        if ((cbuff->sem_used = sem_open(SEM_USED, 0)) == SEM_FAILED) {
            sem_close(cbuff->sem_free);
            free(cbuff);
            return NULL;
        }
        if ((cbuff->sem_mutex = sem_open(SEM_MUTEX, 0)) == SEM_FAILED) {
            sem_close(cbuff->sem_free);
            sem_close(cbuff->sem_used);
            free(cbuff);
            return NULL;
        }
    }

    return cbuff;
}

bool close_cbuff(circular_buffer_t *cbuff, bool server) {
    /** Close semaphores as a server */
    if (server) {
        sem_post(cbuff->sem_free);
        sem_post(cbuff->sem_used);
        cbuff->shm->halt = true;
    }

    sem_close(cbuff->sem_free);
    sem_close(cbuff->sem_used);
    sem_close(cbuff->sem_mutex);

    if (server) {
        sem_unlink(SEM_FREE);
        sem_unlink(SEM_USED);
        sem_unlink(SEM_MUTEX);
    }

    /** Closing semaphores and freeing memory of the circular buffer */
    if (!close_shm(cbuff->shm, cbuff->fd, server)) {
        free(cbuff);
        return false;
    }
    free(cbuff);
    return true;
}

bool add_solution(circular_buffer_t *cbuff, const long *edges, size_t size) {
    if (sem_wait(cbuff->sem_mutex) == -1) {
        sem_post(cbuff->sem_mutex);
        return false;
    }

    /** Iterate through all edges and add size to beginning of payload */
    for (int i = 0; i <= size; ++i) {
        if (cbuff->shm->halt) {
            sem_post(cbuff->sem_mutex);
            return false;
        }

        if (sem_wait(cbuff->sem_free) == -1) {
            sem_post(cbuff->sem_mutex);
            return false;
        }

        if (i == 0) {
            cbuff->shm->data[cbuff->shm->write_idx] = size;
        } else {
            cbuff->shm->data[cbuff->shm->write_idx] = edges[i - 1];
        }

        cbuff->shm->write_idx++;
        cbuff->shm->write_idx %= MAX_DATA;
        sem_post(cbuff->sem_used);
    }

    sem_post(cbuff->sem_mutex);
    return true;
}

long read_buffer(circular_buffer_t *cbuff) {
    long data;
    if (sem_wait(cbuff->sem_used) == -1) {
        sem_post(cbuff->sem_used);
        return -1;
    }
    data = cbuff->shm->data[cbuff->shm->read_idx++];
    cbuff->shm->read_idx %= MAX_DATA;
    sem_post(cbuff->sem_free);
    return data;
}

void print_solution_string(circular_buffer_t *cbuff, size_t size) {
    long temp;
    for (int i = 0; i < size; ++i) {
        temp = read_buffer(cbuff);
        if (temp == -1) return;

        if (i % 2 == 0) {
            printf(" %ld", temp);
        } else {
            printf("-%ld", temp);
        }
    }
}

void skip_solution(circular_buffer_t *cbuff, size_t size) {
    long temp;
    for (int i = 0; i < size; ++i) {
        temp = read_buffer(cbuff);
        if (temp == -1) return;
    }
}
