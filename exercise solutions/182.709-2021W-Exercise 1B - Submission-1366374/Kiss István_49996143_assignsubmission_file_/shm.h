/**
 * @file shm.h
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Shared Memory module.
 * @version 0.1
 * @date 2021-11-08
 * @details This module contains the shared memory struct
 *  and all neccessary functions for it.
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "graph.h"

#ifndef SHM_H_
#define SHM_H_

/**
 * @brief shared memory file name
 * 
 */
#define SHM_NAME "/11909408_myshm"
/**
 * @brief buffer size of the shared memory
 * 
 */
#define MAX_DATA (2048 / sizeof(edge))
/**
 * @brief maximum edges per graph
 * 
 */
#define MAX_EDGES 8
/**
 * @brief macro for errors in shared memory
 * 
 */
#define SHM_ERROR -1

/**
 * @brief Structure of the shared memory.
 * 
 */
typedef struct myshm {
    int healthy;
    int write_pos;
    int read_pos;
    edge data[MAX_DATA];
} myshm;

/**
 * @brief Creates a mapped and truncated shared memory.
 * @details The created shared memory can be accessed by processes of the same user
 * and it is both readable and writeable. If the page does not exist, it is created.
 * 
 * 
 * @param myshm shared memory struct, that has buffer and other necessary information
 * @return int 0 on success, -1 otherwise.
 */
int create_shm(myshm** myshm);

/**
 * @brief Opens a truncated shared memory and maps it to the specified myshm struct.
 * @details The accessed shared memory must be already open.
 * 
 * @param shm pointer to shared memory struct, the shared memory is linked to
 * @return int 0 on success, -1 otherwise.
 */
int open_shm(myshm** shm);

/**
 * @brief Closes shared memory for the specified struct.
 * @details It unmaps, closes and unlinks the related shared memory.
 * 
 * @param shm pointer to shared memory struct, the shared memory is linked to
 * @param shmfd file descriptor of shared memory
 * @return int 0 on success, -1 otherwise.
 */
int close_shm(myshm* shm, int shmfd);

#endif
