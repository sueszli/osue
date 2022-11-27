/**
 * @file circular_buffer.h
 * @author David Jahn, 12020634
 * @brief The header file of the circular buffer datatype
 * @version 1.0
 * @date 2021-11-14
 * 
 */

#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#include "graph.h"
/**
 * @brief Defines how much elements can be in the buffer at the same time
 * 
 */
#define MAX_ELEMENTS_IN_BUFFER (8)

/**
 * @brief Buffer consists of array of graphs and termite variable
 * 
 */
typedef struct {
    graph_t graphs[MAX_ELEMENTS_IN_BUFFER];
    bool terminate;
} circular_buffer_t;

#endif