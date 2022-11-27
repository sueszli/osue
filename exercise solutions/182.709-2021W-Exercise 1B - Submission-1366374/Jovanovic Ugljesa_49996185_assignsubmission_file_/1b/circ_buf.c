#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "circ_buf.h"

void circbuf_create(struct circ_buf *cbuf)
{
    int shmfd = shm_open(SHM_NAME_CIRC_BUF, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "%s: Failed to create shared memory: %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }
    cbuf->shmfd = shmfd;

    int r = ftruncate(shmfd, sizeof(struct circ_buf_shm));
    if (r < 0) {
        fprintf(stderr, "%s: Failed to truncate shared memory: %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct circ_buf_shm *shm;
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm == MAP_FAILED) {
        fprintf(stderr, "%s: Failed to mmap circular buffer: %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < DATA_SIZE; i++) shm->data[i] = -1;
    shm->rd_pos = 0;
    shm->wr_pos = 0;
    cbuf->shm = shm;

    cbuf->used = sem_open(SEM_NAME_USED, O_CREAT | O_EXCL, 0600, 0);
    if (cbuf->used == SEM_FAILED) {
        fprintf(stderr, "%s: Failed to create semaphore 'used': %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    cbuf->free = sem_open(SEM_NAME_FREE, O_CREAT | O_EXCL, 0600, MAX_EDGE_SET);
    if (cbuf->free == SEM_FAILED) {
        fprintf(stderr, "%s: Failed to create semaphore 'free': %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    cbuf->writeable = sem_open(SEM_NAME_WRITEABLE, O_CREAT | O_EXCL, 0600, 1);
    if (cbuf->writeable == SEM_FAILED) {
        fprintf(stderr, "%s: Failed to create semaphore 'writeable': %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    cbuf->closing = sem_open(SEM_NAME_CLOSING, O_CREAT | O_EXCL, 0600, 0);
    if (cbuf->closing == SEM_FAILED) {
        fprintf(stderr, "%s: Failed to create semaphore 'closing': %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void circbuf_init(struct circ_buf *cbuf)
{
    int shmfd = shm_open(SHM_NAME_CIRC_BUF, O_RDWR, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "%s: Failed to create shared memory: %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }
    cbuf->shmfd = shmfd;

    struct circ_buf_shm *shm;
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm == MAP_FAILED) {
        fprintf(stderr, "%s: Failed to mmap circular buffer: %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }
    cbuf->shm = shm;

    cbuf->used = sem_open(SEM_NAME_USED, 0);
    if (cbuf->used == SEM_FAILED) {
        fprintf(stderr, "%s: Failed to create semaphore 'used': %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    cbuf->free = sem_open(SEM_NAME_FREE, 0);
    if (cbuf->free == SEM_FAILED) {
        fprintf(stderr, "%s: Failed to create semaphore 'free': %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    cbuf->writeable = sem_open(SEM_NAME_WRITEABLE, 0);
    if (cbuf->writeable == SEM_FAILED) {
        fprintf(stderr, "%s: Failed to create semaphore 'writeable': %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    cbuf->closing = sem_open(SEM_NAME_CLOSING, 0);
    if (cbuf->closing == SEM_FAILED) {
        fprintf(stderr, "%s: Failed to create semaphore 'closing': %s\n", cbuf->program, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int circbuf_read(struct circ_buf *cbuf, int edge_set[EDGE_SIZE * EDGE_SET_SIZE])
{
    int r;

    r = sem_wait(cbuf->used);
    if (r == -1) {
        if (errno == EINTR) return -2;
        return -1;
    }

    memcpy(
        edge_set,
        &cbuf->shm->data[cbuf->shm->rd_pos * EDGE_SIZE * EDGE_SET_SIZE],
        sizeof(int) * EDGE_SIZE * EDGE_SET_SIZE
    );

    r = sem_post(cbuf->free);
    if (r == -1) {
        return -1;
    }

    cbuf->shm->rd_pos += 1;
    cbuf->shm->rd_pos %= MAX_EDGE_SET;

    return 0;
}

int circbuf_write(struct circ_buf *cbuf, int edge_set[EDGE_SIZE * EDGE_SET_SIZE])
{
    int r;

    r = sem_wait(cbuf->free);
    if (r == -1) {
        return -1;
    }

    r = sem_wait(cbuf->writeable);
    if (r == -1) {
        return -1;
    }
    
    memcpy(
        cbuf->shm->data + (cbuf->shm->wr_pos * EDGE_SIZE * EDGE_SET_SIZE),
        edge_set,
        sizeof(int) * EDGE_SIZE * EDGE_SET_SIZE
    );

    r = sem_post(cbuf->used);
    if (r == -1) {
        return -1;
    }

    cbuf->shm->wr_pos += 1;
    cbuf->shm->wr_pos %= MAX_EDGE_SET;

    r = sem_post(cbuf->writeable);
    if (r == -1) {
        return -1;
    }

    return 0;
}

int circbuf_isactive(struct circ_buf *cbuf)
{
    int closing;
    sem_getvalue(cbuf->closing, &closing);

    return closing == 0;
}

void circbuf_deactivate(struct circ_buf *cbuf)
{
    sem_post(cbuf->closing);
}

void circbuf_close(struct circ_buf *cbuf)
{
    int r;

    r = munmap(cbuf->shm, sizeof(struct circ_buf_shm));
    if (r == -1) {
        fprintf(stderr, "%s: error unmaping shared memory: %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing closing of circbuf\n", cbuf->program);
    }

    r = close(cbuf->shmfd);
    if (r == -1) {
        fprintf(stderr, "%s: error closing shared memory: %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing closing of circbuf\n", cbuf->program);
    }

    r = sem_close(cbuf->used);
    if (r == -1) {
        fprintf(stderr, "%s: error closing semaphore 'used': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing closing of circbuf\n", cbuf->program);
    }

    r = sem_close(cbuf->free);
    if (r == -1) {
        fprintf(stderr, "%s: error closing semaphore 'free': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing closing of circbuf\n", cbuf->program);
    }

    r = sem_close(cbuf->writeable);
    if (r == -1) {
        fprintf(stderr, "%s: error closing semaphore 'writeable': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing closing of circbuf\n", cbuf->program);
    }

    r = sem_close(cbuf->closing);
    if (r == -1) {
        fprintf(stderr, "%s: error closing semaphore 'closing': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing closing of circbuf\n", cbuf->program);
    }
}

void circbuf_destroy(struct circ_buf *cbuf)
{
    int r;

    // Be sure to deactivate circbuf
    // so that the generators also shut down
    circbuf_deactivate(cbuf);

/*
    // wait till all circbuf elements are free
    // and not used.
    // This is done in order to prevent generators
    // waiting for the free semaphore, while the supervisor
    // is already closed
    // implemented with sem_timedwait
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != -1) {
        // generators have 1 seconds to finish their work
        ts.tv_sec += 2;
        while (1) {
            r = sem_timedwait(cbuf->used, &ts);
            if (r == -1) {
                if (errno == ETIMEDOUT) {
                    fprintf(stderr, "%s: stopped waiting for generators to finish\n", cbuf->program);
                    break;
                }
                fprintf(stderr, "%s: error while waiting for used semaphore: %s\n", cbuf->program, strerror(errno));
                fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
                break;
            }
            sem_post(cbuf->free);
        }
    }
*/

    r = munmap(cbuf->shm, sizeof(struct circ_buf_shm));
    if (r == -1) {
        fprintf(stderr, "%s: error unmaping shared memory: %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = close(cbuf->shmfd);
    if (r == -1) {
        fprintf(stderr, "%s: error closing shared memory: %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = shm_unlink(SHM_NAME_CIRC_BUF);
    if (r == -1) {
        fprintf(stderr, "%s: error unlinking shared memory: %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = sem_close(cbuf->used);
    if (r == -1) {
        fprintf(stderr, "%s: error closing semaphore 'used': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = sem_close(cbuf->free);
    if (r == -1) {
        fprintf(stderr, "%s: error closing semaphore 'free': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = sem_close(cbuf->writeable);
    if (r == -1) {
        fprintf(stderr, "%s: error closing semaphore 'writeable': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = sem_close(cbuf->closing);
    if (r == -1) {
        fprintf(stderr, "%s: error closing semaphore 'closing': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = sem_unlink(SEM_NAME_USED);
    if (r == -1) {
        fprintf(stderr, "%s: error unlinking semaphore 'used': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = sem_unlink(SEM_NAME_FREE);
    if (r == -1) {
        fprintf(stderr, "%s: error unlinking semaphore 'free': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = sem_unlink(SEM_NAME_WRITEABLE);
    if (r == -1) {
        fprintf(stderr, "%s: error unlinking semaphore 'writeable': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }

    r = sem_unlink(SEM_NAME_CLOSING);
    if (r == -1) {
        fprintf(stderr, "%s: error unlinking semaphore 'closing': %s\n", cbuf->program, strerror(errno));
        fprintf(stderr, "%s: continuing destruction of circbuf\n", cbuf->program);
    }
}

