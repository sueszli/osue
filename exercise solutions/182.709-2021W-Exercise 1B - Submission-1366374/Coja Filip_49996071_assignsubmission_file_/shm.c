/**
 * @file shm.c
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 11.11.2021
 */

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include "shm.h"

#define SHM_NAME "/12023141_shm"

shared_memory_t *open_shm(int *fd, bool server) {
    int flags = server ? O_RDWR | O_CREAT : O_RDWR;

    /** create and/or open the shared memory object */
    *fd = shm_open(SHM_NAME, flags, 0600);
    if (*fd == -1) {
        return NULL;
//        fprintf(stderr, "Error creating or opening shared memory file at %s \n", SHM_NAME);
//        exit(EXIT_FAILURE);
    }

    if (server) {
        /** set the size of the shared memory */
        if (ftruncate(*fd, sizeof(shared_memory_t)) < 0) {
            shm_unlink(SHM_NAME);
            close(*fd);
            return NULL;
//            fprintf(stderr, "Error allocating size of shared memory file at %s \n", SHM_NAME);
//            exit(EXIT_FAILURE);
        }
    }

    /** map shared memory object */
    shared_memory_t *shm;
    shm = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
    if (shm == MAP_FAILED) {
        shm_unlink(SHM_NAME);
        close(*fd);
        return NULL;
//        fprintf(stderr, "Error mmap allocating size of shared memory file at %s \n", SHM_NAME);
//        exit(EXIT_FAILURE);
    }

    if (server) {
        shm->read_idx = 0;
        shm->write_idx = 0;
        shm->halt = false;
    }
    return shm;
}

bool close_shm(shared_memory_t *shm, int fd, bool server) {
    bool status = true;

    /** unmap shared memory */
    if (munmap(shm, sizeof(*shm)) == -1) {
        status = false;
//        fprintf(stderr, "Error unmapping shared memory %s \n", SHM_NAME);
//        exit(EXIT_FAILURE);
    }

    /** close */
    if (close(fd) == -1) {
        status = false;
//        fprintf(stderr, "Error closing shared memory %s \n", SHM_NAME);
//        exit(EXIT_FAILURE);
    }

    if (server) {
        if (shm_unlink(SHM_NAME) == -1) {
            status = false;
//            fprintf(stderr, "Error unlinking shared memory %s \n", SHM_NAME);
//            exit(EXIT_FAILURE);
        }
    }
    return status;
}
