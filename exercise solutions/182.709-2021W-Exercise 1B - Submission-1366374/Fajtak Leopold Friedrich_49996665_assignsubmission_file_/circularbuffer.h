/**
 * @file circularbuffer.h
 * @author Leopold Fajtak (e0152033@student.tuwien.ac.at)
 * @brief Toolset for circular buffer implementation
 * @version 0.2
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "graph.h"
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define CB_NAME "/01525033_circular_buffer"
#define CIRC_BUFFER_SIZE (16)
#define FREE_SEM "/01525033_free_sem"
#define USED_SEM "/01525033_used_sem"
#define WRITE_SEM "/01525033_write_sem"

struct circularBuffer {
    edge graphs[CIRC_BUFFER_SIZE][MAX_EDGES];
    int sizes[CIRC_BUFFER_SIZE];
    int readIndex;
    int writeIndex;
    bool stop;
};

//For server
/**
 * @brief Initializes a circular buffer
 * @return 0 if no error has occurred, else -1
 *
 */
int init_circ_buf(void);

/**
 * @brief Cleans up a circular buffer after use
 * @return 0 if no error has occurred, else -1
 * 
 */
int cleanup_circ_buf(void);

/**
 * @brief Reads an entry from the circular buffer and returns the obtained information in a graph struct
 * 
 * @return graph* obtained information. !!This is dynamically allocated memory and needs to be freed!!
 */
graph* readBuf(void);

//For client
/**
 * @brief opens a preexisting circular buffer
 * @return 0 if no error has occurred, else -1
 * 
 */
int open_circ_buf(void);

/**
 * @brief closes a circular buffer
 * @return 0 if no error has occurred, else -1
 * 
 */
int close_circ_buf(void);

/**
 * @brief Writes a graph to the circular buffer
 * 
 * @param g Graph to be copied over
 * @return int returns 0 on success, -1 on failure (errno is written), and -2 if all generator processes are told to terminate
 */
int writeToBuf(graph* g);
#endif
