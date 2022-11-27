/**
 * @file supervisor.h
 * @author Stefan Hettinger <e11909446@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Header file.
 * 
 * @details This header file is used in the first assignment (B) called "fb_arc_set".
 * It is used to handle includes, name and size constraints of the shared memory,
 * names of the semaphores as well as a simple data structure for the circular buffer.
 **/

#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <regex.h>

#include <stdint.h>
#include <math.h>
#include <time.h>
#include <limits.h>

//Shared Memory
#define SHM_NAME "/11909446_shm"
#define BUFFER_SIZE (10) //size of the circular buffer

//Semaphores
#define FREE_SEM "/11909446_free"
#define USED_SEM "/11909446_used"
#define BUSY_SEM "/11909446_busy"

/**
 * @brief a single vertex
 * @details represents a single vertex of a graph
 */
typedef long vertex;

/**
 * @brief a single directed edge
 * @details an edge is represented like: u->v
 * @param u vertex u
 * @param v vertex v
 */
typedef struct {
    vertex u;
    vertex v;
} edge;

/**
 * @brief simple array to store edges
 * @details 'edges' stores multiple edges and 'nr_of_edges' can store the total number of stored edges
 * @param edges can store up to 8 edges
 * @param nr_of_edges used to remember the number of edges
 */
typedef struct {
    edge edges[8]; //Assignment says 'You may choose a limit as low as 8 edges.'
    size_t nr_of_edges;
} edge_array;

/**
 * @brief represents the circular buffer
 * @details the circular buffer stores possible solutions, as well as indices and management information
 * @param fb_arc_sets stores the feedback arc sets (up to 'BUFFER_SIZE' many)
 * @param best the best minimum so far
 * @param next_read the next position to read from
 * @param next_write the next position to write to
 * @param nr_of_gen number of registered generators
 * @param term 0 if running, 1 if generators should terminate
 */
typedef struct {
    edge_array fb_arc_sets[BUFFER_SIZE];
    unsigned int best;
    unsigned int next_read;
    unsigned int next_write;
    unsigned int nr_of_gen;
    unsigned int term;
} circular_buffer;
