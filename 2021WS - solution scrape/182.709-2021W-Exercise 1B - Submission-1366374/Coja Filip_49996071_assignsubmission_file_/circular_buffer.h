/**
 * @file circular_buffer.h
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 11.11.2021
 *
 * @brief Provides functions for adding solutions or printing the solutions from the buffer
 *
 * @details This part of the codebase is the one which changed the most by
 * far especially print_solution_string and read_solution_size.
 */

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <semaphore.h>
#include "shm.h"

/** Circular Buffer which uses the shared memory */
typedef struct {
    shared_memory_t *shm;
    sem_t *sem_free;
    sem_t *sem_used;
    sem_t *sem_mutex;
    int fd;
} circular_buffer_t;

/**
 * @brief Adds a solution (flattened edge array) to the circular buffer
 * @details The protocol used to add and read from the buffer must have some type of marker, since we are not saving
 * characters to the buffer but numbers.
 *
 * A solution in the buffer looks something like this:
 * array_size | data | data | data ...
 *
 * So for example
 * 4 | 20 | 30 | 10 | 5 => 20-30 10-5
 *
 * @param cbuff Circular buffer which should be written to
 * @param edges Flattened array of edges which should be added
 * @param size Size of flattened array
 * @return Status if solution was added or not
 */
bool add_solution(circular_buffer_t *cbuff, const long *edges, size_t size);

/**
 * @brief Reads one element from the buffer
 * @details Since the method uses semaphores, there can be errors.
 *
 * If -1 is returned, there was an error with the semaphores.
 *
 * @param cbuff Circular buffer to read from
 * @return Data read from the buffer
 */
long read_buffer(circular_buffer_t *cbuff);

/**
 * @brief Gets and prints (size_t size) elements from the buffer.
 *
 * @param cbuff Circular buffer to be read from.
 * @param size Amount of elements to be printed
 */
void print_solution_string(circular_buffer_t *cbuff, size_t size);

/**
 * @brief Gets (size_t size) elements from the buffer and discards them.
 *
 * @param cbuff Circular buffer to be read from.
 * @param size Amount of elements to be discarded
 */
void skip_solution(circular_buffer_t *cbuff, size_t size);

/**
 * @brief Opens circular buffer and sets up semaphores and shared memory
 * @details When finished, has to be closed with close_cbuff().
 *
 * @param server Boolean which clarifies if the circular buffer is opened by a server or a client.
 * @return NULL or a circular buffer.
 */
circular_buffer_t *open_cbuff(bool server);

/**
 * @brief Closes circular buffer opened by open_cbuff().
 *
 * @param cbuff The circular buffer to be closed.
 * @param server Boolean which clarifies if the circular buffer is closed by a server or a client.
 * @return Status if circular buffer has been closed properly.
 */
bool close_cbuff(circular_buffer_t *cbuff, bool server);

#endif
