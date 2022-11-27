/**
 * @file graph.h
 * @author David Jahn, 12020634
 * @brief The header file of the graph datatype
 * @version 1.0
 * @date 2021-11-14
 * 
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#define MAX_EDGES (100)

/**
 * @brief Node of a graph, has an id, a color and may be colored or not
 * 
 */
typedef struct
{
    int id;
    short color;    //0-2
    bool isColored; // true of false
} node_t;

/**
 * @brief Edge of a graph, consists of 2 nodes
 * 
 */
typedef struct
{
    node_t from;
    node_t to;
} edge_t;

/**
 * @brief Graph, has an array of edges an a length of the array
 * 
 */
typedef struct
{
    int edges_len;
    edge_t edges[MAX_EDGES];
} graph_t;

/**
 * @brief Returns if the node is colored in the given graph
 * 
 * @return true if the node is colored
 * @return false if the node is not colored
 */
bool isColored(graph_t *, int);

/**
 * @brief Mehtod used for coloring a specific node in a specific color
 * 
 */
void color(graph_t *, int, int);

/**
 * @brief Returns true if a node with this id exists in given graph, false otherwise
 * 
 * @return true if a node with this id exists in given graph
 * @return false otherwise
 */
bool nodeExists(graph_t *, int);

/**
 * @brief Get the Node object
 * 
 * @return node_t the Node object with specified id
 */
node_t getNode(graph_t *, int);

/**
 * @brief Returns a graph with a specific edge removed
 * 
 * @return graph_t 
 */
graph_t removeEdge(graph_t , edge_t );

#endif