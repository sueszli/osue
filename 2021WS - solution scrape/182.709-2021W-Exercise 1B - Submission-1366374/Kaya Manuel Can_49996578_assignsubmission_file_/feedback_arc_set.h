/**
 * @file feedback_arc_set.h
 * @author Manuel Can Kaya 12020629
 * @brief Header file for assignment 1B Feeback arc set.
 * @details Header file contains all necessary include-directives and macros for the supervisor and
 * generator to work with shared memory and semaphores, as well as all complementary header files. It also
 * defines typedefs and structs for vertex, edge, fb_arc_set_sol and circular_buffer.
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef FEEDBACK_ARC_SET_H
#define FEEDBACK_ARC_SET_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> // For exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h> // For getopt(), getpid().
#include <limits.h> // For INT_MIN and INT_MAX.
#include <ctype.h> // For isdigit().
#include <string.h> // For string functions.
#include <time.h> // For time().
#include <assert.h> // For defensive programming.
#include <errno.h> // For errno, strerror().
#include <signal.h> // For signal handling.
#include <fcntl.h> // For 0_* constants.
#include <semaphore.h> // For semaphores.
#include <sys/types.h> // For pid_t.
#include <sys/stat.h> // For mode constants.
#include <sys/mman.h> // For shared memory.

/** Name macro for shared memory. */
#define SHM_NAME  "/12020629_NAME"

/** Name macro for semaphore indicating used space in circular buffer. */
#define SEM_USED  "/12020629_USED"

/** Name macro for semaphore indicating free space in circular buffer. */
#define SEM_FREE  "/12020629_FREE"

/** Name macro for semaphore/mutex to control write-access to the circular buffer. */
#define SEM_MUTEX "/12020629_MUTEX"


/** Size of the circular buffer. */
#define BUFFER_SIZE 25

/** Maximum size of a feedback arc set solution. */
#define MAX_EDGES 8


/**
 * @brief Vertex typdef.
 * @details Defining vertex as a long type.
 * 
 */
typedef long vertex;

/**
 * @brief Edge struct.
 * @details Struct representing a directed edge with a from_vertex and to_vertex.
 * 
 */
typedef struct {
    vertex from_vertex;
    vertex to_vertex;
} edge;

/**
 * @brief Feedback arc set solution struct.
 * @details Struct representing a feedback arc set solution. Contains the edges
 * of the solution and its size.
 * 
 */
typedef struct {
    edge edges[MAX_EDGES];
    int size;
} fb_arc_set_sol;

/**
 * @brief Cicular buffer struct.
 * @details Struct representing a circular buffer. Contains a boolean variable 'terminate'
 * that tells the generators to terminate, the feedback arc set solutions found so far in
 * an array 'solutions', the next position to be read 'read_pos' and the next position to
 * write a solution to 'write_pos'. 
 * 
 */
typedef struct {
    bool terminate;
    fb_arc_set_sol solutions[BUFFER_SIZE];
    int read_pos;
    int write_pos;
} circular_buffer;


/**
 * @brief Prints an error to stderr.
 * @details Prints a formated string to stderr for error messages. The error message
 * includes the program name 'program_name', a brief message 'brief_msg' and a more detailed
 * message 'detail_msg'.
 * 
 * @param program_name Name of the program.
 * @param brief_msg Brief error message.
 * @param detail_msg Detailed error message.
 */
void error_msg(const char *program_name, const char *brief_msg, const char *detail_msg);

#endif // FEEDBACK_ARC_SET_H