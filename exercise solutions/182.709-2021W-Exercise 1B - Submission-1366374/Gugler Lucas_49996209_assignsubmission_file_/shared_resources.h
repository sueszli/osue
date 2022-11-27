/**
 * @file supervisor.c
 * @author Lucas Gugler 12019849
 * @date 4.11.2021
 *
 * @brief Header file for shared resources
 * @details This file defines several structurs for storing the different feedback arc sets
 * All used libraries and the locations for the shared memory and sephamores are also defined in this file
 **/
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>
#include <limits.h>

/** location shared memory*/
#define SHMLOCATION "/12019849shm"
/** number of feedback arc sets in shared memory*/
#define BUFFER_SIZE 20
/** location semaphore used for writing access*/
#define SEMUNSET "/12019849unset"
/** location semaphore used for reading access*/
#define SEMSET "/12019849set"
/** location semaphore used for 1 max writer*/
#define SEMWRITE "/12019849write"

/**
 * @brief Represents an edge.
 * @details An edge defined by two vertices saved as long.
 */
typedef struct
{
    /**first vertex*/
    long u;
    /**second vertex*/
    long v;
} edge;

/**
 * @brief Represents a feedback arc set.
 * @details An feedback arc set defined by the contained edges.
 */
typedef struct
{
    /**edges in arc set*/
    edge edges[8];
    /**number of set edges*/
    int counter;
} feedback_arc_set;

/**
 * @brief Represents the circular buffer used in shared memory.
 * @details A circular buffer for simultaneous reading and writing of feedback arc sets,
 *passed between supervisor and generator
 */
typedef struct
{
    /**container for feedback arc sets solutions*/
    feedback_arc_set data[BUFFER_SIZE];
    /**next readable position*/
    int read_position;
    /**next writable postion*/
    int write_position;
    /**flag for stop and shutting down program*/
    int stop;
    /**number of registered generator programs*/
    int numgenerators;

} circular_buffer;