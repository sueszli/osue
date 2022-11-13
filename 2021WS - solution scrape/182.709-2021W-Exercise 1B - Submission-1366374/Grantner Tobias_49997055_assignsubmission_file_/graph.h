/**
 * @file graph.h
 * @author Tobias Grantner (12024016)
 * @brief This class represents a graph with a maximum of GRAPH_MAX_EDGE_COUNT and defines functions to handle a graph.
 * @date 2021-11-11
 */

#ifndef GRAPH_H /* include guard */
#define GRAPH_H

#include <stdio.h>

/**
 * @brief Max amount of edges allowed in a graph
 */
#define GRAPH_MAX_EDGE_COUNT (8)

/**
 * @brief Struct representing edges of a graph
 */
typedef struct {
    int x;
    int y;
} edge_t;

/**
 * @brief Struct representing a graph with a maximum of GRAPH_MAX_EDGE_COUNT edges
 */
typedef struct {
    size_t count;
    edge_t edges[GRAPH_MAX_EDGE_COUNT];
} graph_t;

/**
 * @brief Create a edge object
 * 
 * @param x First node of the edge
 * @param y Second node of the edge
 * @return edge_t The created edge object
 */
edge_t create_edge(int x, int y);

/**
 * @brief Create a graph object
 * 
 * @param edges The edges that should be in the graph
 * @param count The amount of edges in the edge array
 * @return graph_t The created graph
 */
graph_t create_graph(const edge_t * edges, size_t count);

/**
 * @brief Get the nodes from an array of edges
 * 
 * @param destination Where the edges should be stored to, has to be at leaset 2 * edgeCount big
 * @param edges The edges where the nodes should be extracted from
 * @param edgeCount The number of edges in the edges array
 * @return size_t The number of nodes extracted from the edges
 */
size_t get_nodes(int * destination, const edge_t * edges, size_t edgeCount);

/**
 * @brief Prints all edges in an array
 * 
 * @param output Where the result should be printed to
 * @param edges The edges in an array
 * @param count The number of edges in the array
 * @return int < 0 if error, otherwise 0
 */
int print_edges(FILE * output, const edge_t * edges, size_t count);

#endif