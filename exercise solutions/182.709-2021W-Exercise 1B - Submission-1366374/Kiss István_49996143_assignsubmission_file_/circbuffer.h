/**
 * @file circbuffer.h
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Circular buffer module.
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <semaphore.h>
#include "shm.h"
#include "graph.h"

#ifndef CIRCBUFFER_H_
#define CIRCBUFFER_H_

/**
 * @brief Free space semaphore name.
 * 
 */
#define SEM_FREE "/11909408_sem_1"
/**
 * @brief Used space semaphore name.
 * 
 */
#define SEM_USED "/11909408_sem_2"
/**
 * @brief Exclusive write semaphore name.
 * 
 */
#define SEM_WRITE "/11909408_sem_3"
/**
 * @brief Buffer error macro
 * 
 */
#define BUFFER_ERROR -1

/**
 * @brief Structure of the circular buffer.
 * 
 */
typedef struct mybuffer {
    int shmfd;
    sem_t* used_sem;
    sem_t* free_sem;
    sem_t* write_sem;
    myshm* shm;
} mybuffer;

/**
 * @brief Create a buffer object
 * @details The created object is linked to shared memory.
 * All semaphores are opened in the struct.
 * If the shared memory or semaphores already exist this function fails.
 * 
 * @param buffer struct the shared memory and semaphores are linked to
 * @return int 0 on success, otherwise -1
 */
int create_buffer(mybuffer* buffer);

/**
 * @brief Reads at most MAX_EDGES number of edges from the buffer.
 * @details This is a blocking operation. It decrements the used space semaphore
 * and increments the free space semaphore.
 * 
 * @param buffer struct holding the shared memory and semaphores
 * @param edge pointer to memory the edges are copied to.
 * @return int 0 on success, otherwise -1
 */
int read_buffer(mybuffer* buffer, edge* edge);

/**
 * @brief Opens the buffer for the specified buffer struct.
 * @details Both shared memory and semaphores must exist, otherwise this function fails.
 * It links the shared memory and semaphores to the buffer.
 * 
 * @param buffer struct the shared memory and semaphores are linked to
 * @return int 0 on success, otherwise -1
 */
int open_buffer(mybuffer* buffer);

/**
 * @brief Writes edges of this graph with exactly MAX_EDGES number of edges to the buffer.
 * This is a blocking operation. It waits for the exclusive write access and
 * decrements the free space semaphore and increments the used space semaphore.
 * 
 * @param buffer struct holding the shared memory and semaphores
 * @param graph holding exactly MAX_EDGES number of edges
 * @return int 0 on success, otherwise -1
 */
int write_buffer(mybuffer* buffer, graph* graph);

/**
 * @brief Closes the buffer.
 * @details This closes the shared memory and semaphores in the right order.
 * 
 * @param buffer struct holding the shared memory and semaphores
 * @return int 0 on success, otherwise -1
 */
int close_buffer(mybuffer* buffer);

#endif
