/**
 * @file buffer.c
 * @author Jakub Fidler 12022512
 * @date 13 Nov 2021
 * @brief implementation of buffer.h. defines a circular buffer for solutions to the 3-coloring problem
 **/

#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include "buffer.h"

#define PROGRAM_NAME "buffer"

#define SHM_NAME "/12022512_three_color"
#define SEM_FREE "/12022512_sem_free"
#define SEM_USED "/12022512_sem_used"
#define SEM_WRITE "/12022512_sem_write"

sem_t *sem_free;  // for keeping track of the free fields in the buffer
sem_t *sem_used;  // for keeping track of the used fields in the buffer
sem_t *sem_write; // for atomic write operations to the buffer

typedef struct buffer
{
    volatile bool terminated;
    volatile unsigned int read_pos;
    volatile unsigned int write_pos;
    int data[BUFFER_SIZE][SOLUTION_MAX_NUM_EDGES][2];
} buffer;

// size of one solution in the buffer in bytes as it is saved in buffer->data
const int SOLUTION_SIZE = SOLUTION_MAX_NUM_EDGES * 2 * sizeof(int);

static void handle_signal(int signal);
static bool safe_sem_wait(sem_t *sem);
static bool safe_sem_post(sem_t *sem);

// buffer struct that is mapped to shared memory to be accessible from multiple processes
// contains solutions to the 3-coloring problem
buffer *buf;

bool buffer_setup(void)
{
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
        return false;

    if (ftruncate(shmfd, sizeof(buffer)) < 0)
        return false;

    buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buf == MAP_FAILED)
        return false;

    if (close(shmfd) == -1)
        return false;

    sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    if (sem_free == SEM_FAILED)
        return false;

    sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    if (sem_used == SEM_FAILED)
        return false;

    sem_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);
    if (sem_write == SEM_FAILED)
        return false;

    buf->terminated = false;
    buf->read_pos = 0;
    buf->write_pos = 0;

    struct sigaction sig_handler;
    memset(&sig_handler, 0, sizeof(sig_handler));
    sig_handler.sa_handler = handle_signal;

    sigaction(SIGINT, &sig_handler, NULL);
    sigaction(SIGTERM, &sig_handler, NULL);

    return true;
}

bool buffer_open(void)
{
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1)
        return false;

    buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buf == MAP_FAILED)
        return false;

    if (close(shmfd) == -1)
        return false;

    sem_free = sem_open(SEM_FREE, 0, 0600);
    if (sem_free == SEM_FAILED)
        return false;

    sem_used = sem_open(SEM_USED, 0, 0600);
    if (sem_used == SEM_FAILED)
        return false;

    sem_write = sem_open(SEM_WRITE, 0, 0600);
    if (sem_write == SEM_FAILED)
        return false;

    return true;
}

bool buffer_close(void)
{
    if (munmap(buf, sizeof(*buf)) == -1)
        return false;

    if (sem_close(sem_free) == -1)
        return false;

    if (sem_close(sem_used) == -1)
        return false;

    if (sem_close(sem_write) == -1)
        return false;

    return true;
}

bool buffer_clean_up(void)
{
    bool result = true;
    if (sem_unlink(SEM_FREE) != 0)
        result = false;

    if (sem_unlink(SEM_USED) != 0)
        result = false;

    if (sem_unlink(SEM_WRITE) == -1)
        result = false;

    if (shm_unlink(SHM_NAME) == -1)
        result = false;

    return result;
}

static bool safe_sem_wait(sem_t *sem)
{
    while (!buffer_has_terminated())
    {
        if (sem_wait(sem) == 0)
            break;
        else
        {
            if (errno != EINTR)
                return false;
        }
    }
    if (buffer_has_terminated())
        return false;
    return true;
}

static bool safe_sem_post(sem_t *sem)
{
    while (!buffer_has_terminated())
    {
        if (sem_post(sem) == 0)
            break;
        else
        {
            if (errno != EINTR)
                return false;
        }
    }
    if (buffer_has_terminated())
        return false;
    return true;
}

bool buffer_write(int edges[SOLUTION_MAX_NUM_EDGES][2])
{
    if (!safe_sem_wait(sem_free))
        return false;

    if (!safe_sem_wait(sem_write))
        return false;

    memcpy(buf->data[buf->write_pos], edges, SOLUTION_SIZE);
    buf->write_pos = (buf->write_pos + 1) % BUFFER_SIZE;

    if (!safe_sem_post(sem_write))
        return false;

    if (!safe_sem_post(sem_used))
        return false;

    return true;
}

bool buffer_read(int edges[SOLUTION_MAX_NUM_EDGES][2])
{
    if (!safe_sem_wait(sem_used))
        return false;

    memcpy(edges, buf->data[buf->read_pos], SOLUTION_SIZE);
    buf->read_pos = (buf->read_pos + 1) % BUFFER_SIZE;

    if (!safe_sem_post(sem_free))
        return false;

    return true;
}

static void release_stuck_clients(void)
{
    // clients can become stuck if the server terminates while they are in a sem_wait call

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        sem_post(sem_free);
    }

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        sem_post(sem_write);
    }
}

void buffer_terminate(void)
{
    buf->terminated = true;
    release_stuck_clients();
}

bool buffer_has_terminated(void)
{
    return buf->terminated;
}

static void handle_signal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
        buffer_terminate();
}