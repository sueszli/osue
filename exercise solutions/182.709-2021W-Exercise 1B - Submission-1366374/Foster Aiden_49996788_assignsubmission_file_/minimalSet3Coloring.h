/**
 * @file minimalSet3Coloring.h
 * @author Aiden Foster 11910604
 * @date 12.11.2021
 *
 * @brief General code for shared memory in minimalSet3Coloring
*/

#ifndef MINIMAL_SET_3_COLORING_H
#define MINIMAL_SET_3_COLORING_H

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_NAME "/11910604_shm_minimalSet3Coloring_buffer"
#define BUFFER_MAX_DATA 256
#define BUFFER_END_SYMBOL -1
#define TERMINATE_FLAG_NAME "/11910604_shm_minimalSet3Coloring_terminate_flag"
#define TERMINATE_FLAG_T unsigned char

#define SEM_FREE_BUFFER "/11910604_sem_free_buffer"
#define SEM_USED_BUFFER "/11910604_sem_used_buffer"
#define SEM_WR_BUFFER "/11910604_sem_wr_buffer"

/**
 * @brief An edge containing two verticies v1 and v2
**/
struct edge {
    /**
     * @brief First vertex of directed edge
    **/
    int v1;
    /**
     * @brief Second vertex of directed edge
    **/
    int v2;
};

/**
 * @brief An edge containing two verticies v1 and v2
**/
typedef struct edge edge_t;

/**
 * @brief Circular buffer for use as shared memory
**/
struct sharedCircularBuffer {
    /**
     * @brief Index at which to write next
    **/
    unsigned int w;
    /**
     * @brief Index at which to read next
    **/
    unsigned int r;
    /**
     * @brief Data of circular buffer as array of edges
    **/
    edge_t edges[BUFFER_MAX_DATA];
};

#endif
