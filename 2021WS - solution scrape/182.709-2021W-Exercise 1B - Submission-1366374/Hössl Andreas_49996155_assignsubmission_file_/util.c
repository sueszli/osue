/**
 * @file util.c
 * @author Andreas Hoessl <e11910612@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Utility module.
 *
 * Implementation of the header module 'util.h'.
 */

#include "util.h"

void exit_error(char *message) {
    fprintf(stderr, "[%s] ERROR: %s: %s\n", myprog, message, strerror(errno));
    exit(EXIT_FAILURE);
}

void s_wait(sem_t *semaphore) {
    if (sem_wait(semaphore) == -1) {
        if (errno != EINTR) {
            exit_error("sem_wait failed");
        }
    }
}

void s_post(sem_t *semaphore) {
    if (sem_post(semaphore) == -1) {
        if (errno != EINTR) {
            exit_error("sem_post failed");
        }
    }
}
