/**
 * @file graph.h
 * @author Jannis Adamek (11809490)
 * @date 2021-11-14
 *
 * @brief Defines datastructors and common methods used to model graphs.
 **/

#ifndef GRAPH
#define GRAPH

#include <stdlib.h>

/** Defines how many edges a solution set can have at most. */
#define MAX_EDGES_IN_EXCESS_SET 10

/** Edge data type, v1 is <= v2 */
typedef struct _edge {
  size_t v1; /** v1 is lower than v2 */
  size_t v2;
} Edge;

/**
 * EdgeSet is the struct that gets filled.
 * next_index is the next available index (= the size of the EdgeSet).
 */
typedef struct _edge_set {
  Edge edges[MAX_EDGES_IN_EXCESS_SET];
  size_t next_index;
} EdgeSet;

/**
 * Graph is used by the generator process.
 */
typedef struct _graph {
  Edge *edge_arr;
  size_t next_edge_index;
  size_t number_of_edges;

  int *edge_color_arr;
  size_t highest_vertex_num;
} Graph;

/**
 * Allocates a new graph when the space is known.
 * @param number_of_edges This is known because the graph is given as argument.
 */
Graph *malloc_graph(size_t number_of_edges);

/**
 * Allocate space for the array, that array of chosen colors.
 * @brief Alloc the color array, this functino should be called after all edges
 * have been added to the graph, because then graph->highest_vertex_num will be
 * set at this point.
 * @param graph pointer where the color_arr is NULL.
 */
void malloc_edge_color_arr(Graph *graph);

/**
 * Add an edge to a graph
 * @brief Create new edge from string, e.g from "12-421"
 * @details the edge must be freed by the user of this function
 * @param graph A valid graph (e.g not NULL)
 * @param edge_represent A string represeneting the value of an edge
 */
void add_edge_to_graph(Graph *graph, const char *edge_represent);

/**
 * Color a graph randomly
 * @brief Choose random colors for all edges of the graph.
 * @param graph A valid graph pointer
 */
void color_graph_randomly(Graph *graph);

/**
 * Find the set of edges, that must be removed from the colored graph for it to
 * be 3-colored.
 * @brief Remove all edges where both vertices have the same color.
 */
void get_excess_edges(Graph *graph, EdgeSet *edge_set);

/**
 * Free a graph
 * @brief free the graph and its suparrays.
 * @param graph can be a valid graph pointer or NULL.
 */
void free_graph(Graph *graph);

/**
 * Free edge set
 * @brief Frees a valid EdgeSet
 * @param edge_set must a valid pointer or NULL.
 */
void free_edge_set(const EdgeSet *edge_set);

/**
 * Print the set of edges onto stdout.
 * @param edge_set to be printed.
 */
void print_edge_set(const EdgeSet *edge_set);

#endif