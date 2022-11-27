/**
 * @file graph.h
 * @author Leopold Fajtak (e0152033@student.tuwien.ac.at)
 * @brief Toolset for working with graphs
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef GRAPH_H
#define GRAPH_H

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Maximum amount of edges per graph
 * 
 */
#define MAX_EDGES (4096)
/**
 * @brief maximum amount of nodes per graph
 * 
 */
#define MAX_NODES (64)

struct edge{
    unsigned int from;
    unsigned int to;
}typedef edge;

struct graph{
    edge edges[MAX_EDGES];
    int numEdges;
}typedef graph;

/**
 * @brief Generates an edge object from a string conforming to the pattern %d-%d
 * 
 * @param s 
 * @return edge* Caution: This is dynamically allocated memory and needs to be freed!
 */
edge* scanEdge(char* s);

/**
 * @brief Add an edge resembling e to the graph
 * 
 * @param g 
 * @param e 
 * @return int returns 0 on success, and -1 on failure (errno is set)
 */
int addEdge(graph* g, edge e);

/**
 * @brief prints graph g to stdout
 * 
 * @param g 
 */
void printGraph(graph* g);
//indices from edge need to be accessible in edge_permutation

/**
 * @brief Assesses whether the edge e makes the given coloring invalid. !!The node indices of e need to be valid indices of coloring!!
 * 
 * @param edge_permutation 
 * @param e 
 * @return true e is a forward edge
 * @return false e is not a forward edge
 */
bool edgeMakesColoringInvalid(int* coloring, edge e);

/**
 * @brief Returns the maximal node index present in g. (For it to make sense, all indices must be positive)
 * 
 * @param g 
 * @return int 
 */
int numNodes(graph* g);

/**
 * @brief initializes a graph objects with zeroes
 * 
 * @param g 
 */
void initGraph(graph* g);

#endif
