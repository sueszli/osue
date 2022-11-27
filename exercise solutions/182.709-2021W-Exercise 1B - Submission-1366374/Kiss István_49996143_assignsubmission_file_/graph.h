/**
 * @file graph.h
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Graph module specification.
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdlib.h>
#include <stdint.h>

#ifndef GRAPH_H_
#define GRAPH_H_

/**
 * @brief Struct for saving edges.
 * 
 */
typedef struct Edge {
    uint32_t from;
    uint32_t to;
} edge;

/**
 * @brief Struct for saving graph.
 * 
 */
typedef struct Graph {
    edge* edges;
    uint32_t buffer_size;
    uint32_t num_of_edges;
} graph;

/**
 * @brief Adds an edge to the graph.
 * @details It allocates memory if necessary.
 * If it is unsafe then allows to add duplicate edges.
 * 
 * @param dst graph to add the edge to.
 * @param edge edge to insert
 * @param unsafe allow duplicate
 * @return graph* pointer to graph, NULL if edge is a duplicate or allocation fails.
 */
graph* add_edge(graph* dst, edge* edge, int unsafe);

/**
 * @brief Returns a feedback arc set from the specified graph.
 * @details The possible feedback arc set is calculated using a randomized algorithm.
 * It allocates memory for the edges if needed.
 * 
 * @param src source graph
 * @param dst feedback arc set destination
 * @return graph* pointer to feedback arc set, NULL on fail.
 */
graph* find_feeback_arc_set(graph* src, graph* dst);

/**
 * @brief Prints edges of the graph in a specific format.
 * @details Prints to stdout.
 * 
 * @param src graph containing edges to print
 */
void print_edges(graph* src);

#endif
