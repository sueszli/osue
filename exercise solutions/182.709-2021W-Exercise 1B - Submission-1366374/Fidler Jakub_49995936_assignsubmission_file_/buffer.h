/**
 * @file buffer.h
 * @author Jakub Fidler 12022512
 * @date 13 Nov 2021
 * @brief defines a circular buffer for solutions to the 3-coloring problem
 **/

#include <semaphore.h>

#ifndef shm_h
#define shm_h

#define BUFFER_SIZE (20)           // max amount of results in buffer
#define SOLUTION_MAX_NUM_EDGES (8) // max amount of edges in result

/**
 * @brief Initializes the buffer
 * @details Must be used before other buffer_ functions.
 * @return true if no error occured, else false
 */
bool buffer_setup(void);

/**
 * @brief Opens the buffer.
 * @details Must be used after buffer_setup.
 * @return true if no error occured, else false
 */
bool buffer_open(void);

/**
 * @brief Reads a solution from the buffer.
 * @param edges will contain the read solution
 * @return true if no error occured, else false
 */
bool buffer_read(int edges[SOLUTION_MAX_NUM_EDGES][2]);

/**
 * @brief Writes a solution to the buffer.
 * @param edges will be written to the buffer
 * @return true if no error occured, else false
 */
bool buffer_write(int edges[SOLUTION_MAX_NUM_EDGES][2]);

/**
 * @brief Returns if the buffer has terminated.
 * @return true if the buffer has terminated, else false
 */
bool buffer_has_terminated(void);

/**
 * @brief Terminates the buffer
 * @details Sets a flag that can be checked using buffer_has_terminated. Does some clean-up.
 */
void buffer_terminate(void);

/**
 * @brief Closes the buffer
 * @details Closes the buffer for the process that calls the function.
 * @return true if the buffer has terminated, else false
 */
bool buffer_close(void);

/**
 * @brief Cleans up buffer resources
 * @details Cleans up resources of the buffer. Must be called after buffer_close.
 * @return true if the buffer has terminated, else false
 */
bool buffer_clean_up(void);

#endif