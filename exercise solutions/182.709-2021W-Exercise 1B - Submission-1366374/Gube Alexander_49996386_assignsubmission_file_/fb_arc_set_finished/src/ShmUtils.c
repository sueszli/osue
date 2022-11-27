/**
 * @file ShmUtils.c
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief implementation of the utility functions for shared memories
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/mman.h>                   //for shared memory
#include <fcntl.h>

#include "ErrorHandling.h"
#include "ShmUtils.h"

/**
 * @details unmap a shm and returns 0 on success, otherwise writes error to stderr and return -1
 */
int unmap(struct fbArc *shmLoc, char* progName) {
    if(munmap(shmLoc, bufferSize * sizeof(struct fbArc)) < 0) {
        failedWithError(progName, "failed to unmap the circular buffer", 0);
        return -1;
    }
    return 0;
}

/**
 * @details close a shm and returns 0 on success, otherwise writes error to stderr and return -1
 */
int closeSHM(int fd, char* progName) {
    if(close(fd) < 0) {
        failedWithError(progName, "failed to close fd of circular buffer", 0);
        return -1;
    }
    return 0;
}

/**
 * @details unlink a shm and returns 0 on success, otherwise writes error to stderr and return -1
 */
int unlinkSHM(char *name, char* progName) {
    if(shm_unlink(name) < 0) {
        failedWithError(progName, "failed to close the circular buffer", 0);
        return -1;
    }
    return 0;
}

/**
 * @details container method which cleans up SHM without terminating on error
 */
void cleanupSHM(char *name, struct fbArc *shmLoc, int fd, char* progName, int doUnlink) {
    unmap(shmLoc, progName);
    closeSHM(fd, progName);
    if(doUnlink > 0) {
        unlinkSHM(name, progName);
    }
}
