/**
 * Shared memory module for generators
 * @file shmgen.h
 * @author Marvin Flandorfer, 52004069
 * @date 05.11.2021
 * 
 * @brief This is a shared memory module for the generators.
 * This module opens, maps and unmaps the shared memory for generators.
 */

#ifndef SHMGEN_H
#define SHMGEN_H

#include "myshm.h"

/**
 * Function for opening the shared memory
 * @brief This function opens the shared memory and maps it accordingly.
 * 
 * @return Returns a pointer to the shared memory on success. Returns NULL on failure.
 */
shm_t *open_shared_memory(void);

/**
 * Function for the cleanup of the shared memory
 * @brief This function unmaps the shared memory object.
 * 
 * @param shm Pointer to the shared memory object.
 * @return Returns 0 on success. Returns -1 on failure.
 */
int cleanup_shared_memory(shm_t *shm);

#endif