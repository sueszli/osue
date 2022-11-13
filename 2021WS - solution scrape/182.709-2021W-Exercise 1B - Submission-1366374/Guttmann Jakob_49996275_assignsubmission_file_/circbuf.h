/**
 * @file circbuf.h
 * @author Jakob Guttmann 11810289 <e11810289@student.tuwien.ac.at>
 * @brief in this header there is the struct for circular buffer and for arcset defined
 * also contains makros for the names of the shared objects and an error handling message
 * @date 11.11.2021
 * 
 * 
 */
#ifndef __CIRCBUF_H__
#define __CIRCBUF_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>

#include "graph.h"

/**
 * @brief defines name of writing semaphore for circular buffer
 * 
 */
#define SEM_WR "/11810289_sem_wr"
/**
 * @brief defines name of reading semaphore for circular buffer
 * 
 */
#define SEM_RD "/11810289_sem_rd"
/**
 * @brief defines name of semaphore for mutual exclusion
 * 
 */
#define SEM_MX "/11810289_sem_mx"
/**
 * @brief defines name of shared memory object
 * 
 */
#define SHM_NAME "/11810289_shmbuf"

/**
 * @brief size of circular buffer is 32 entries
 * 
 */
#define SIZE_BUF (32)

/**
 * @brief macro for errorhandling, x is the message to be printed and exits with EXIT_FAILURE
 * 
 */
#define ERRORHANDLING(x)                                                                      \
    {                                                                                         \
        fprintf(stderr, "[%s] ERROR: cannot %s\nReason: %s\n", argv[0], #x, strerror(errno)); \
        exit(EXIT_FAILURE);                                                                   \
    }

/**
 * @brief struct for an feedback arc set, saves the number of edges and an array of fixed size of 8 
 * therefore we can exactly define the size of the circular buffer
 * 
 */
struct arcset
{
    int num_edges;
    Edge edges[8];
};
typedef struct arcset arcset_t;

/**
 * @brief have an array of saved feedback arcset which serves as an circular buffer
 * active flag indicates if shared memory is active
 * wr_index where next index is written
 * rd_index where next index is read
 * minimum the global minimum in this buffer (for performance reasons)
 * 
 */
struct circularbuf
{
    arcset_t buffer[SIZE_BUF];
    char active;
    char wr_index;
    char rd_index;
    char minimum;
    int randomSeed;
};
typedef struct circularbuf circbuf;

#endif