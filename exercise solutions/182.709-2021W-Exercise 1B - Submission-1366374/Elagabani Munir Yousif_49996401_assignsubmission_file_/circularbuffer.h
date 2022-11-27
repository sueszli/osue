/**
 * @file circularbuffer.h
 * @author Munir Yousif Elagabani <e12022518@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief header file for the circularbuffer
 **/

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define BUFFER_LENGTH 50
#define MAX_EDGES_FB_ARC 8

#define SHM_NAME "/12022518_shm"
#define FREE_SEM_NAME "/12022518_free_sem"
#define USED_SEM_NAME "/12022518_used_sem"
#define WRITE_SEM_NAME "/12022518_write_sem"

#include <stdbool.h>
#include <stdio.h>

/**
 * @brief edge structure with from and to vertex
 *
 */
typedef struct edge {
  int from;
  int to;
} edge_t;

/**
 * @brief The solution struct with a fixed sized selection array and a variable
 * that represents the correct number of edges in the array. At any index >=
 * numEdges the value in the selection array is garbage.
 */
typedef struct fbArc {
  edge_t selection[MAX_EDGES_FB_ARC];
  size_t numEdges;
} fbArc_t;

/**
 * @brief buffer struct for the shared memory containing the array and variables
 * that must be shared between all generators and the supervisor.
 *
 */
typedef struct fbArcBuffer {
  fbArc_t solutions[BUFFER_LENGTH];
  int bestSolutionSize;
  int wr_pos;
  int rd_pos;
} fbArcBuffer_t;

/**
 * @brief writes a solution to the circularbuffer
 * this function blocks when there is no free space left
 * @param val solution to be written into the buffer
 */
void write_buffer(fbArc_t val);

/**
 * @brief read a solution from the circularbuffer
 * this function blocks when there is no used space
 * @return fbArc_t solution from the buffer if an error occurs an invalid
 * solution is returned
 */
fbArc_t read_buffer(void);

/**
 * @brief function for setting up the buffer
 *
 * @param isServer marks the buffer for server use where additional functions
 * are called on setup and cleanup
 * @param progamname saves programname for potential error messages
 * @return Returns 0 on success and -1 on failure
 */
int load_buffer(bool isServer, char *progamname);

/**
 * @brief function for cleaning up the buffer
 * error checking is skipped because the subsequent code doesn't depend on the
 * successful execution of the function.
 *
 */
void cleanup_buffer(void);

/**
 * @brief function for communicating the termination of the supervisor
 * the variable that keeps track of the bestsolutionsize in the buffer gets set
 * to -1 indicating the termination of the supervisor
 */
void terminate(void);

/**
 * @brief function checks for termination of the supervisor
 * the variable that keeps track of the bestsolutionsize in the buffer is set to
 * -1 if the supervisor got terminated.
 * @return true
 * @return false
 */
bool shouldTerminate(void);

/**
 * @brief Get the min solution size
 *
 * @return int current min solution size
 */
int get_min_solution_size(void);

/**
 * @brief Set the min solution size
 *
 * @param size new minimum solution size
 */
void set_min_solution_size(size_t size);

#endif /* CIRCULAR_BUFFER_H */