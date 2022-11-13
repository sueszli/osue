/**
 * @file sharedcircularbuffer.h
 * @author Adrian Chroust <e12023146@student.tuwien.ac.at>
 * @date 12.11.2021
 * @brief Shared circular buffer module.
 * @details This module provides the data structures, shared memory and semaphores needed
 *          for the shared circular buffer to be read from one process and written to by multiple processes.
 */

#ifndef SHARED_CIRCULAR_BUFFER_H
#define SHARED_CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <semaphore.h>
#include <limits.h>

/**
 * @brief The namespace under which the shared circular buffer shared_buffer is allocated.
 */
#define BUFFER_SHM_NAME "/12023146_1b_buffer"

/**
 * @brief The namespace under which the semaphore free_space_sem indicating the free space is located.
 */
#define FREE_SPACE_SEM_NAME "/12023146_1b_free"

/**
 * @brief The namespace under which the semaphore used_space_sem indicating the used space is located.
 */
#define USED_SPACE_SEM_NAME "/12023146_1b_used"

/**
 * @brief The namespace under which the semaphore write_access_sem indicating the mutually exclusive write access is located.
 */
#define WRITE_ACCESS_SEM_NAME "/12023146_1b_write"

/**
 * @brief The amount of unsigned integers that can be stored inside the data property of the buffer buffer_t.
 */
#define BUFFER_DATA_LENGTH 32

/**
 * @brief The unsigned integer which indicates the end of a write.
 * @details This signal is used by the process that reads the data so it knows
 *          that the mutually exclusive write of a writing process has ended.
 */
#define END_OF_WRITE UINT_MAX

/**
 * @brief The type definition of the shared circular buffer.
 * @details It stores an integer which is either 0 or 1 indicating whether the writing processes should quit,
 *          a process count indicating the number of active writing processes,
 *          the write position and the data array of the size BUFFER_DATA_LENGTH.
 */
typedef struct {
    unsigned int quit;
    unsigned int process_count;
    unsigned int write_pos;
    unsigned int data[BUFFER_DATA_LENGTH];
} buffer_t;

/**
 * @brief Holds the id of the current process.
 * @details It is set after the function initialize_shared_circular_buffer and
 *          unset after the function terminate_shared_circular_buffer has been successfully called.
 *          If the shared circular buffer is not set the process_id is 0.
 */
unsigned int process_id;

/**
 * @brief The shared circular buffer which points to shared memory.
 * @details It is set after the function initialize_shared_circular_buffer and
 *          unset after the function terminate_shared_circular_buffer has been successfully called.
 */
buffer_t *shared_buffer;

/**
 * @brief The semaphore indicating the free space,
 *        meaning the amount of unsigned integers that can be written to the buffer data.
 * @details It is set after the function initialize_shared_circular_buffer and
 *          unset after the function terminate_shared_circular_buffer has been successfully called.
 *          Upon creation it is initialized with the BUFFER_DATA_LENGTH.
 */
sem_t *free_space_sem;

/**
 * @brief The semaphore indicating the used space,
 *        meaning the amount of unsigned integers that have been written and not yet read to and from the buffer data.
 * @details It is set after the function initialize_shared_circular_buffer and
 *          unset after the function terminate_shared_circular_buffer has been successfully called.
 *          Upon creation it is initialized with the 0.
 */
sem_t *used_space_sem;

/**
 * @brief The semaphore indicating the mutually exclusive write access,
 *        meaning that only one writing process can write to the shared buffer at a time.
 * @details It is set after the function initialize_shared_circular_buffer and
 *          unset after the function terminate_shared_circular_buffer has been successfully called.
 *          Upon creation it is initialized with the 1.
 */
sem_t *write_access_sem;

/**
 * @brief Initializes the shared circular buffer and all semaphores.
 * @details Usually the reading process creates the shared circular buffer while the writing processes just open it.
 * @param create A boolean value indicating if the shared memory and semaphores have yet to be created or just opened.
 * @return -1 if an error occurred during the initialization, 0 otherwise.
 */
int initialize_shared_circular_buffer(bool create);

/**
 * @brief Terminates the shared circular buffer and all semaphores.
 * @details Usually the reading process removes the shared circular buffer while the writing processes just close it.
 * @param remove A boolean value indicating if the shared memory and semaphores should be removed or just closed.
 * @return -1 if an error occurred during the initialization, 0 otherwise.
 */
int terminate_shared_circular_buffer(bool remove);

#endif // SHARED_CIRCULAR_BUFFER_H
