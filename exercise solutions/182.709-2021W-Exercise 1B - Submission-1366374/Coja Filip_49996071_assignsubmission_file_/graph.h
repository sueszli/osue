/**
 * @file graph.h
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 09.11.2021
 *
 * @brief Helper methods and structs to work with graphs.
 * @details Also implements the method to search for which edges should be deleted.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <glob.h>

/** Structs which should manage the structure of a graph (Edges could be implemented via linked lists, but it's easier without it) */
typedef enum COLOR {
    red = 0, green = 1, blue = 2
} color_e;

/** Node */
typedef struct {
    color_e color;
    long id;
} node_t;

/** Edge */
typedef struct {
    node_t *node1;
    node_t *node2;
} edge_t;

/** Graph */
typedef struct {
    node_t *nodes;
    size_t node_count;
    edge_t *edges;
    size_t edge_count;
} graph_t;

/**
 * @brief Deletes a graph.
 * @details Deallocates all dynamically created arrays with malloc() and finally itself.
 * @param graph Graph to be deleted
 */
void delete_graph(graph_t *graph);

/**
 * @brief Colors all nodes randomly.
 * @details Method uses rand() which is initalized with srand(time + getpid())
 * @param graph Graph to color.
 */
void color_randomly(graph_t *graph);

/**
 * @brief Tries to find node with a specific Id.
 * @param graph The graph where the node should be found.
 * @param id The identifier which should be searched for.
 * @return NULL or the node it found.
 */
node_t *find_node_by_id(graph_t *graph, long id);

/**
 * @brief Creates a copy of a graph.
 * @details Method was used as a helper before the supervisor etc. was implemented.
 *
 * @param graph The graph to be copied.
 * @return New graph which was copied. Has to be freed.
 */
graph_t *copy(graph_t *graph);

/**
 * @brief Gets all edges which have to be deleted so that the graph becomes 3colorable.
 *
 * @param graph Graph to be analyzed.
 * @param buffer Buffer to write the edges which should be deleted to.
 * Must be at least twice the amount of edges in the graph since it's flattened.
 * @return Amount of edges to be removed.
 */
long get_deletion_edges(graph_t *graph, long *buffer);

#endif
