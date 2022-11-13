/**
 * @file SemUtils.c
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief implementation of the utility functions for semaphores
 *
 **/

#include "ErrorHandling.h"
#include "SemUtils.h"

/**
 * @details closes a semaphore and returns 0 on success, otherwise writes error to stderr and return -1
 */
int closeSEM(sem_t *sem, char* progName) {
    if(sem_close(sem) < 0) {
        failedWithError(progName, "failed to close Sem", 0);
        return -1;
    }
    return 0;
}

/**
 * @details unlinks a semaphore and returns 0 on success, otherwise writes error to stderr and return -1
 */
int unlinkSEM(char *name, char* progName) {
    if(sem_unlink(name) < 0) {
        failedWithError(progName, "failed to close semaphore", 0);
        return -1;
    }
    return 0;
}

/**
 * @details container function of the two above method without listing on success of those functions
 */
void cleanupSEM(sem_t *sem, char *name, char *progName) {
    closeSEM(sem, progName);
    unlinkSEM(name, progName);
}
