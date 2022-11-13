/**
 * @file circbuffer.c
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Implementation of the circular buffer module.
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "circbuffer.h"

/**
 * @brief macro to check shared mem errors
 * 
 */
#define CHECK(x) do { \
    if ((x) < 0) { \
        return BUFFER_ERROR; \
    } \
} while (0) \

/**
 * @brief macro to check semaphore errors
 * 
 */
#define CHECK_SEM(x) do { \
    if ((x) == SEM_FAILED) { \
        return BUFFER_ERROR; \
    } \
} while(0) \

/**
 * @brief Decrements the specified semaphore count times.
 * @details It also checks whether the operation was successful.
 * 
 * @param sem semaphore to be decremented
 * @param count number of times the semaphore is decremented
 * @return int 0 on success, otherwise -1
 */
static int decrement_sem(sem_t* sem, int count) {
    for (int i = 0; i < count; i++) {
        CHECK(sem_wait(sem));
    }
    return 0;    
}

/**
 * @brief Increments the specified semaphore count times.
 * @details It also checks whether the operation was successful.
 * 
 * @param sem semaphore to be incremented
 * @param count number of times the semaphore is incremented
 * @return int 0 on success, otherwise -1
 */
static int increment_sem(sem_t* sem, int count) {
    for (int i = 0; i < count; i++) {
        CHECK(sem_post(sem));
    }
    return 0;
}

int create_buffer(mybuffer* buffer) {
    CHECK(buffer->shmfd = create_shm(&(buffer->shm)));
    buffer->shm->healthy = 1;
    CHECK_SEM(buffer->free_sem = (sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_DATA)));
    CHECK_SEM(buffer->used_sem = (sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0)));
    CHECK_SEM(buffer->write_sem = (sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1)));
    return 0;
}

int read_buffer(mybuffer* buffer, edge* edg) {
    CHECK(decrement_sem(buffer->used_sem, MAX_EDGES));
    for (int i = 0; i < MAX_EDGES; i++) {
        edg[i] = buffer->shm->data[buffer->shm->read_pos];
        buffer->shm->read_pos += 1;
        buffer->shm->read_pos %= MAX_DATA;
    }
    CHECK(increment_sem(buffer->free_sem, MAX_EDGES));
    return 0;
}

int open_buffer(mybuffer* buffer) {
    CHECK(buffer->shmfd = open_shm(&(buffer->shm)));
    CHECK_SEM(buffer -> free_sem = (sem_open(SEM_FREE, O_EXCL, 0600, MAX_DATA)));
    CHECK_SEM(buffer -> used_sem = (sem_open(SEM_USED, O_EXCL, 0600, 0)));
    CHECK_SEM(buffer->write_sem = (sem_open(SEM_WRITE, O_EXCL, 0600, 1)));
    return 0;
}

int write_buffer(mybuffer* buffer, graph* graph) {
    CHECK(sem_wait(buffer->write_sem));
    CHECK(decrement_sem(buffer->free_sem, MAX_EDGES));
    for (int i = 0; i < MAX_EDGES; i++) {
        buffer->shm->data[buffer->shm->write_pos] = graph->edges[i];
        buffer->shm->write_pos += 1;
        buffer->shm->write_pos %= MAX_DATA;
    }
    CHECK(increment_sem(buffer->used_sem, MAX_EDGES));
    CHECK(sem_post(buffer->write_sem));
    return 0;
}

int close_buffer(mybuffer* buffer) {
    CHECK(close_shm(buffer->shm, buffer->shmfd));
    CHECK(sem_close(buffer->free_sem));
    CHECK(sem_close(buffer->used_sem));
    CHECK(sem_unlink(SEM_FREE));
    CHECK(sem_unlink(SEM_USED));
    CHECK(sem_unlink(SEM_WRITE));
    return 0;
}
