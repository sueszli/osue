/**
 * @file util.c
 * @author Alexander Gschnitzer (01652750) <e1652750@student.tuwien.ac.at>
 * @date 21.10.2021
 *
 * @brief Utility functions intended to prevent code duplication.
 * @details Collection of utility functions, including usage and error function.
 * Associated header file also contains type definitions and macros that are used globally.
 */

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <semaphore.h>

/**
 * @brief Prog_name as the first parameter of argv array and valid arguments of program.
 * @details Defined in supervisor.c and generator.c. The variable arguments is null in the case of supervisor program.
 */
extern const char *prog_name, *arguments;

/**
 * @brief Shared memory flags to define access of file
 * @details Defined in supervisor.c and generator.c.
 */
extern const int shm_flags, sem_flags;

/**
 * @brief Circular buffer acting as the mapped shared memory object.
 * @details Defined in supervisor.c and generator.c.
 */
extern cb_t *buffer;

/**
 * @brief Semaphores that store information about free and used space in circular buffer respectively.
 * sem_access is used to prevent concurrent access to circular buffer.
 */
extern sem_t *sem_free, *sem_used, *sem_access;

void usage(void) {
    fprintf(stderr, "Usage: %s %s\n", prog_name, arguments != NULL ? arguments : "");
    exit(EXIT_FAILURE);
}

void error(const char *message) {
    fprintf(stderr, "[%s] ERROR %s: %s\n", prog_name, message, strerror(errno));
    exit(EXIT_FAILURE);
}

void print_solution(const solution_t *solution) {
    if (solution->removed == 0) {
        printf("[%s] The graph is 3-colorable!\n", prog_name);
        return;
    }

    printf("[%s] Solution found by removing %d edge%s:", prog_name, solution->removed, solution->removed != 1 ? "s" : "\0");
    for (int i = 0; i < solution->removed; i++) {
        printf(" %d-%d", solution->edges[i].v1, solution->edges[i].v2);
    }
    printf("\n");
}

void init_buffer(int is_supervisor) {
    // create shared memory object
    int shm = shm_open(SHM_NAME, shm_flags, 0600);
    if (shm == -1) {
        error("Creation or opening of the shared memory object failed");
    }

    if (is_supervisor) {
        // set size of shared memory object to the size of the circular buffer
        if (ftruncate(shm, sizeof(cb_t)) < 0) {
            error("Setting the size of shared memory failed");
        }
    }

    // map shared memory object to circular buffer, or exit the program if an error occurs
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (buffer == MAP_FAILED) {
        error("Mapping of shared memory object failed");
    }

    // close shared memory object since it is not needed anymore and the setup of the buffer is completed
    if (close(shm) == -1) {
        error("Closing of shared memory object failed");
    }

    // buffer should not be null after initialization of shared memory
    assert(buffer != NULL);
}

void clear_buffer(int is_supervisor) {
    // unmap shared memory object
    if (munmap((void *) buffer, sizeof(*buffer)) == -1) {
        error("Unmapping of the shared memory object failed");
    }

    // removal of shared memory object should only be executed once by the supervisor
    if (is_supervisor && shm_unlink(SHM_NAME) == -1) {
        error("Removal of shared memory object failed");
    }
}

void init_sem(int is_supervisor) {
    // differentiate between supervisor or generator initialization
    if (is_supervisor) {
        // create semaphores

        /*
         * sem_free equals to MAX_SIZE since buffer is initially empty
         * sem_used equals to 0 since buffer is initially empty
         * sem_access equals to 1 since only one client at a time is allowed to access buffer
         */
        sem_free = sem_open(SEM_FREE, sem_flags, 0600, MAX_SIZE);
        sem_used = sem_open(SEM_USED, sem_flags, 0600, 0);
        sem_access = sem_open(SEM_ACCESS, sem_flags, 0600, 1);
    } else {
        // open previously created semaphores
        sem_free = sem_open(SEM_FREE, sem_flags);
        sem_used = sem_open(SEM_USED, sem_flags);
        sem_access = sem_open(SEM_ACCESS, sem_flags);
    }

    // check of correct semaphores is deliberately at the end to prevent unnecessary code duplication
    if (sem_free == SEM_FAILED) {
        error("Creation or opening of SEM_FREE failed");
    } else if (sem_used == SEM_FAILED) {
        error("Creation or opening of SEM_USED failed");
    } else if (sem_access == SEM_FAILED) {
        error("Creation or opening of SEM_ACCESS failed");
    }
}

void clear_sem(int is_supervisor) {
    // close semaphores, exit program if process fails
    if (sem_close(sem_free) == -1) {
        error("Closing of SEM_FREE failed");
    } else if (sem_close(sem_used) == -1) {
        error("Closing of SEM_USED failed");
    } else if (sem_close(sem_access) == -1) {
        error("Closing of SEM_ACCESS failed");
    }

    // removal of semaphores should only be executed once by the supervisor
    if (is_supervisor) {
        // remove semaphore for free space in buffer, exit program if process fails
        if (sem_unlink(SEM_FREE) == -1) {
            error("Removal of SEM_FREE failed");
        } else if (sem_unlink(SEM_USED) == -1) {
            error("Removal of SEM_USED failed");
        } else if (sem_unlink(SEM_ACCESS) == -1) {
            error("Removal of SEM_ACCESS failed");
        }
    }
}
