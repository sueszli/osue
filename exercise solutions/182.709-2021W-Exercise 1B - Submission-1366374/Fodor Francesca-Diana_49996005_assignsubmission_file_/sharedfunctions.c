/**
 * @file sharedfunctions.c
 * @author Fodor Francesca Diana, 11808223
 * @date 08.11.2020
 * @brief This file contains the implementation of the sharedfunctions module
 */

#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdarg.h>
#include "sharedfunctions.h"

#define PROGRAMNAME "./sharedfunctions.c"
#define ERR_EXIT(msg) fprintf(stderr, "[%s] %s error: %s\n", PROGRAMNAME, msg, strerror(errno)); exit(EXIT_FAILURE);
#define USER_ERROR(msg) fprintf(stderr, "%s\n", msg); exit(EXIT_FAILURE);


void sem_unlink_all(int n, ...) {
    va_list valist;
    va_start(valist, n);
    for (int i = 0; i < n; i++) {
        sem_unlink(va_arg(valist,
        char*));
    }
}

void sem_close_all(int n, ...) {
    va_list valist;
    va_start(valist, n);
    for (int i = 0; i < n; i++) {
        sem_close(va_arg(valist, sem_t * ));
    }
}
