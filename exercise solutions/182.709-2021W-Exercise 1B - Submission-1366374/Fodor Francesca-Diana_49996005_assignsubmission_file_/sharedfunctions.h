/**
 * @file sharedfunctions.h
 * @author Fodor Francesca Diana, 11808223
 * @date 08.11.2021
 * @brief This module contains structures to store solutions to the 3-coloring problem
 * into a circular buffer. There are multiple method declarations to make sure entries
 * are written to the correct position, to print a solution and to clean up resources.
 */
#ifndef SHAREDFUNCTIONS
#define SHAREDFUNCTIONS

#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

//macros for filenames of the shared memory object and the semaphores
#define SHM_NAME "/11808223_3_color"
#define SEM_RD "/11808223_3_color_free"
#define SEM_PST "/11808223_3_color_used"
#define SEM_SYNC "/11808223_3_color_sync"

//the number of entries the circular buffer contains
#define NUM_ELEMS 20

//the maximum number of edges to be considered for a solution
#define MAX_LENGTH 8

//the maximum number of nodes that can be passed to a generator
#define MAX_NODES (UINT8_MAX+1)

/**
 * enumeration colors
 * @brief This is an enumeration of the three possible colors for the 3-coloring problem, with which the nodes can be colored.
 */
typedef enum {
    RED, GREEN, BLUE
} colors;

/**
 * struct
 * @brief A struct to store an entry for the 3-coloring problem. It holds the result which is transmitted to the
 * supervisor, namely the number of edged and the edges stored in an arrray.
 */
typedef struct {
    unsigned int edges[16];
    unsigned int numberOfEdges;
} color3;

/**
 * myshm struct
 * @brief Shared memory data. The struct implements the circular buffer. It contains the read index and the write index
 * from the circular buffer. The state indicates when the program should be stopped and is changed by the signal handler.
 * It also contains the solution of the 3-coloring problem made of a struct color3 array of 8 edges used by the supervisor.
 */
struct myshmentry {
    unsigned int state;
    unsigned int readIndex;
    unsigned int writeIndex;
    color3 solution[MAX_LENGTH];
};

/**
 * myshm struct
 * @brief Shared memory data. The struct implements the circular buffer. It contains the read index and the write index
 * from the circular buffer. The state indicates when the program should be stopped and is changed by the signal handler.
 * It also contains the solution of the 3-coloring problem made of a struct color3 array of 20 edges used by the generators.
 */
struct myshm {
    unsigned int state;
    unsigned int readIndex;
    unsigned int writeIndex;
    color3 solution[NUM_ELEMS];
};

/**
 * @brief This function takes multiple filenames of semaphores as input and calls sem_unlink for
 * each.
 * @param n The number of filenames passed
 * @param ... Every other parameter is considered to be a filename of a semaphore
 */
void sem_unlink_all(int n, ...);

/**
 * @brief This function takes multiple references to semaphores and calls sem_close on each
 * @param n The number of semaphores passed
 * @param ... Every other parameter is considered to be a reference to an opened semaphore
 */
void sem_close_all(int n, ...);

#endif
