/**
 * @file shared.h
 * @author Joé Kieffer <e11814254@student.tuwien.ac.at>
 * @date 08.11.2021
 *
 * @brief Header of the main files
 *
 * @details This file contains the names of the semaphores and the shared memory file.
 *  Furthermore the shared memory buffer is defined inclusive the sizes.
 *  The write position in the buffer is too defined.
 *
 */
#ifndef SHARED_H
#define SHARED_H

#include <signal.h>

#define SEM_USED "/11814254_sem_used"
#define SEM_FREE "/11814254_sem_free"
#define SEM_MUTEX "/11814254_sem_mutex"

#define SHM_NAME "/11814254_3coloring"
#define MAX_DATA (100)
#define MAX_EDGE (100)

struct myshm{
    volatile sig_atomic_t quit;
    int writePos;
    int data[MAX_DATA][MAX_EDGE][2];
    int ndata[MAX_DATA];
};

#endif