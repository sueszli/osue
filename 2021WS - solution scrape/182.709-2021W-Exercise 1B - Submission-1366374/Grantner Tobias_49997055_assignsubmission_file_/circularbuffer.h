/**
 * @file graph.h
 * @author Tobias Grantner (12024016)
 * @brief This class represents a circularbuffer which holds up to CB_MAX_SIZE graphs and defines methods to insert and remove graphs in the FIFO principle.
 * @date 2021-11-11
 */

#ifndef CIRCULARBUFFER_H /* include guard */
#define CIRCULARBUFFER_H

#include "graph.h"
#include <stdio.h>

/**
 * @brief The maximum number of graphs in the circularbuffer
 */
#define CB_MAX_SIZE (64)

/**
 * @brief The name of the semaphore used to regulate free spaces in the circularbuffer
 */
#define CB_SEM_FREE "/12024016_sem_free"

/**
 * @brief The name of the semaphore used to regulate used spaces in the circularbuffer
 */
#define CB_SEM_USED "/12024016_sem_used"

/**
 * @brief Struct representing the circularbuffer
 */
typedef struct {
    size_t read_head;
    size_t write_head;
    graph_t data[CB_MAX_SIZE];
} circularbuffer_t;

/**
 * @brief Inserts a graph at the end of the circularbuffer.
 * 
 * @param cb The circularbuffer the graph should be inserted into
 * @param graph The graph to be inserted
 */
void cb_push(circularbuffer_t * cb, graph_t graph);

/**
 * @brief Removes the first graph from a circularbuffer and returns it
 * 
 * @param cb The circularbuffer from wich the first graph should be removed and returned
 * @return graph_t The graph that was removed
 */
graph_t cb_pop(circularbuffer_t * cb);

#endif