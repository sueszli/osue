/**
 * @file shm.c
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Shared memory module implementation.
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "shm.h"

/**
 * @brief Maps shared memory using the specified file descriptor.
 * @details The initialized Shared Memory page is both readable and writeable and changes are shared.
 * 
 * @param shm shared memory struct containing its details
 * @param shmfd shared memory file descriptor
 * @return int 0 if success, otherwise SHM_ERROR
 */
static int map_shm(myshm** shm, int shmfd) {
    *shm = mmap(NULL, sizeof(myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (*shm == MAP_FAILED) {
        return SHM_ERROR;
    }
    return 0;
}

/**
 * @details uses the macro SHM_ERROR
 */
int create_shm(myshm** myshm) {
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        return SHM_ERROR;
    }
    if (ftruncate(shmfd, sizeof(struct myshm)) < 0) {
        return SHM_ERROR;
    }

    if (map_shm(myshm, shmfd) == SHM_ERROR) {
        return SHM_ERROR;
    }
;
    return shmfd;
}

int open_shm(myshm** shm) {
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_EXCL, 0600);
    if (shmfd == -1) {
        return SHM_ERROR;
    }
    if (map_shm(shm, shmfd) != SHM_ERROR) {
        return shmfd;
    }
    return SHM_ERROR;
}

int close_shm(myshm* shm, int shmfd) {
    if (munmap(shm, sizeof(myshm)) < 0) {
        return SHM_ERROR;
    }
    if (close(shmfd) < 0) {
        return SHM_ERROR;
    }
    if (shm_unlink(SHM_NAME) < 0) {
        return SHM_ERROR;
    }
    return 0;
}
