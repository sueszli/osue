/**
 * Shared memory module for the supervisor program.
 * @file shmsupvis.h
 * @author Marvin Flandorfer, 52004069
 * @date 06.11.2021
 * 
 * @brief This is the shared memory modul for supervisor.
 * This module is used by the supervisor for creating, mapping, unmaping and unlinking the shared memory.
 */

#ifndef SHMSUPVIS_H
#define SHMSUPVIS_H

#include "myshm.h"

/**
 * Function for creating a shared mermory object
 * @brief Creates a shared memory object.
 * @details Additionally sets up all variables from the struct myshm.
 * 
 * @return Returns a pointer to the shared memory object on success. Returns NULL on failure.
 */
shm_t *create_shared_memory(void);

/**
 * Function for the shared memory cleanup.
 * @brief This function unmaps and unlinks the shared memory object.
 * 
 * @param shm Pointer that points to the shared memory object.
 * @return Returns 0 on success and -1 on failure.
 */
int cleanup_shared_memory(shm_t *shm);

#endif