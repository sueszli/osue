#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "circular_buffer.h"

/**
 * @file circular_buffer.c
 * @author Elias GALL - 12019857
 * @brief Implements a circular buffer for use by one (reading) supervisor and multiple (writing) generators,
 *        using shared memory and semaphores.
 * @details Implements a circular buffer using 3 shared semaphores -- one to signal if entries can be read,
 *          one to limit writing capacity, so that no unready entries get overwritten and one to limit how
 *          many processes can write at once. Furthermore the buffer uses shared memory to store it's data.
 *          The supervisor is supposed to create the circular buffer, and set up the semaphores and memory,
 *          while the generators only connect to an existing one.
 * @date 2021-11-02
 */

// server: create, close_after_create
// client: connect, disconnect

/** @brief pointer to the buffer in shared memory */
static circular_buffer_t *buffer = NULL;

/** @brief limits access to writing to one process at a time */
static sem_t *wr_access_sem = NULL;

/** @brief limits the amount of entries that can be written to the size of the buffer */
static sem_t *wr_space_sem = NULL;

/** @brief limits the number of entries that can be read to the amount that has been written */
static sem_t *rd_entries_sem = NULL;

/** @brief file descriptor of the shared memory */
static int shm_fd = 0;


/**
 * @brief closes all semaphores
 * @details Closes all semaphores and does not return immediately on error, as to try and
 *          close all remaining semaphores as well.
 * @details Global variables used: wr_access_sem, wr_space_sem, rd_access_sem
 * @param program_name name of the program (arg[0]) for use in error messages
 * @return 0 ... success
 * @return -1 ... one or more errors occured
 */
static int close_semaphores(const char *program_name);

/**
 * @brief next position for a buffer index
 * @details Generates the index following the given one by a simple addition and modulo operation.
 * @param index a valid index of the buffer
 * @return the index following the one provided, wrapping around
 */
static int nextBufferIndex(int index);

int create(const char *program_name) {
    // create the shared memory
    shm_fd = shm_open(BUFFER_NAME, O_RDWR | O_CREAT, 0600);
    if (shm_fd == -1) {
        // error
        fprintf(stderr, "%s: shm_open failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    int ft_code = ftruncate(shm_fd, sizeof(circular_buffer_t));
    if (ft_code == -1) {
        // error
        printf("%s - %i\n", strerror(errno), errno);
        fprintf(stderr, "%s: ftruncate failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    buffer = mmap(NULL, sizeof(circular_buffer_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buffer == MAP_FAILED) {
        // error
        fprintf(stderr, "%s: mmap failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    buffer->read_pos = 0;
    buffer->write_pos = 0;
    buffer->terminate = 0;

    // create semaphore(s)
    wr_access_sem = sem_open(WRITE_ACCESS_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0600, 1);
    if (wr_access_sem == SEM_FAILED) {
        // error
        fprintf(stderr, "%s: sem_open failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    wr_space_sem = sem_open(WRITE_SPACE_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0600, BUFFER_LENGTH);
    if (wr_space_sem == SEM_FAILED) {
        // error
        fprintf(stderr, "%s: sem_open failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    rd_entries_sem = sem_open(READ_ENTRIES_SEMAPHORE_NAME, O_CREAT | O_EXCL, 0600, 0);
    if (rd_entries_sem == SEM_FAILED) {
        // error
        fprintf(stderr, "%s: sem_open failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    return 0;
}

int close_after_create(const char *program_name) {
    buffer->terminate = 1;
    int error = 0;
    if (shm_unlink(BUFFER_NAME) == -1) {
        fprintf(stderr, "%s: shm_unlink failed - %s\n", program_name, strerror(errno));
        error++;
    }
    // files
    if (shm_fd != 0) {
        if (close(shm_fd) == -1) {
            // error
            fprintf(stderr, "%s: close failed - %s\n", program_name, strerror(errno));
            error++;
        }
        shm_fd = 0;
    }
    if (sem_unlink(WRITE_SPACE_SEMAPHORE_NAME) == -1) {
        //error
        fprintf(stderr, "%s: sem_unlink failed - %s\n", program_name, strerror(errno));
        error++;
    }
    if (sem_unlink(WRITE_ACCESS_SEMAPHORE_NAME) == -1) {
        //error
        fprintf(stderr, "%s: sem_unlink failed - %s\n", program_name, strerror(errno));
        error++;
    }
    if (sem_unlink(READ_ENTRIES_SEMAPHORE_NAME) == -1) {
        //error
        fprintf(stderr, "%s: sem_unlink failed - %s\n", program_name, strerror(errno));
        error++;
    }
    // semaphores
    if (close_semaphores(program_name) == -1) {
        error++;
    }
    return error == 0 ? 0 : -1;
}

int connect(const char *program_name) {
    int fd = shm_open(BUFFER_NAME, O_RDWR, 0);
    if (fd == -1) {
        // error
        fprintf(stderr, "%s: shm_open failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    buffer = mmap(NULL, sizeof(circular_buffer_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        // error
        fprintf(stderr, "%s: mmap failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    if (close(fd) == -1) {
        // error
        fprintf(stderr, "%s: close failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    // map an existing object
    wr_access_sem = sem_open(WRITE_ACCESS_SEMAPHORE_NAME, 0);
    if (wr_access_sem == SEM_FAILED) {
        // error
        fprintf(stderr, "%s: sem_open failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    wr_space_sem = sem_open(WRITE_SPACE_SEMAPHORE_NAME, 0);
    if (wr_space_sem == SEM_FAILED) {
        // error
        fprintf(stderr, "%s: sem_open failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    rd_entries_sem = sem_open(READ_ENTRIES_SEMAPHORE_NAME, 0);
    if (rd_entries_sem == SEM_FAILED) {
        // error
        fprintf(stderr, "%s: sem_open failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    return 0;
}

int disconnect(const char *program_name) {
    int error = 0;
    if (munmap(buffer, sizeof(*buffer)) == -1) {
        // error
        fprintf(stderr, "%s: munmap failed - %s\n", program_name, strerror(errno));
        error++;
    }
    if (close_semaphores(program_name) == -1) {
        // error
        error++;
    }
    return error == 0 ? 0 : -1;
}

void set_terminate(const int terminate) {
    buffer->terminate = terminate;
}

int get_terminate(void) {
    return buffer->terminate;
}

int write_buffer(buffer_entry_t *e, const char *program_name) {
    if (buffer->terminate == 1) {
        // terminate
        return -1;
    }
    // wait until space to write becomes free
    if (sem_wait(wr_space_sem) == -1) {
        // error or interrupt
        if (buffer->terminate != 1) {
            fprintf(stderr, "%s: sem_wait failed - %s\n", program_name, strerror(errno));
        }
        return -1;
    }
    if (buffer->terminate == 1) {
        // terminate
        return -1;
    }
    // get exclusive write access to the buffer
    if (sem_wait(wr_access_sem) == -1) {
        // error or interrupt
        fprintf(stderr, "%s: sem_wait failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    if (buffer->terminate == 1) {
        // terminate
        return -1;
    }

    // write to buffer
    memcpy(&(buffer->entries[buffer->write_pos]), e, sizeof(buffer_entry_t));
    buffer->write_pos = nextBufferIndex(buffer->write_pos);

    // release write access to the buffer
    if (sem_post(wr_access_sem) == -1) {
        // error
        fprintf(stderr, "%s: sem_post failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    // an additional entry can be read
    if (sem_post(rd_entries_sem) == -1) {
        // error
        fprintf(stderr, "%s: sem_post failed - %s\n", program_name, strerror(errno));
        return -1;
    }
    return 0;
}

buffer_entry_t *read_buffer(const char *program_name) {
    // wait until an entry is ready to be read
    if (sem_wait(rd_entries_sem) == -1) {
        // error
        if (buffer->terminate != 1) {
            fprintf(stderr, "%s: sem_wait failed - %s\n", program_name, strerror(errno));
        }
        return NULL;
    }

    // read from buffer
    buffer_entry_t *entry = malloc(sizeof(buffer_entry_t));
    if (entry == NULL) {
        // error
        fprintf(stderr, "%s: malloc failed - %s\n", program_name, strerror(errno));
        return NULL;
    }
    if (memcpy(entry, &(buffer->entries[buffer->read_pos]), sizeof(buffer_entry_t)) == NULL) {
        // error
        fprintf(stderr, "%s: memcpy failed - %s\n", program_name, strerror(errno));
        return NULL;
    }
    buffer->read_pos = nextBufferIndex(buffer->read_pos);

    // one more entry can be written
    if (sem_post(wr_space_sem) == -1) {
        // error
        fprintf(stderr, "%s: sem_post failed - %s\n", program_name, strerror(errno));
        return NULL;
    }
    return entry;
}

static int close_semaphores(const char *program_name) {
    int error = 0;
    if (sem_close(wr_access_sem) == -1) {
        // error
        fprintf(stderr, "%s: sem_close failed - %s\n", program_name, strerror(errno));
        error++;
    }
    if (sem_close(wr_space_sem) == -1) {
        // error
        fprintf(stderr, "%s: sem_close failed - %s\n", program_name, strerror(errno));
        error++;
    }
    if (sem_close(rd_entries_sem) == -1) {
        // error
        fprintf(stderr, "%s: sem_close failed - %s\n", program_name, strerror(errno));
        error++;
    }
    return error == 0 ? 0 : -1;
}

static int nextBufferIndex(int index) {
    return (index + 1) % BUFFER_LENGTH;
}