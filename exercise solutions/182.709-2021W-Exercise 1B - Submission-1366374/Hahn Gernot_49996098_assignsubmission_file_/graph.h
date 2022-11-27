/**
 * @file graph.h
 * @author Gernot Hahn 01304618
 * @date 14.11.2021
 *
 * @brief Provides useful graph types and functions.
 *
 * @details This module provides type definitions and functions
 * useful for working with graphs. It defines types for an edge
 * and a feedback arc set. It also contains functions to parse edges,
 * calculate the number of edges of a fb arc set and to calculate 
 * a permutation of a set of vertices.
 */

#define MAX_SET (8)

#ifndef GRAPH_H
#define GRAPH_H

typedef struct edge {
	int v1;
	int v2;
} edge_t;

typedef struct fb_arc {
	edge_t edges[MAX_SET];
} fb_arc_set;

#endif

/**
 * @brief Parses an edge.
 * @details This function takes a string representation of an edge in the
 * form of "v1-v2", where v1 is the outgoing vertex and v2 the incoming
 * vertex, and saves the vertices into the specified array.
 * @param String representation of the edge.
 * @param Pointer to an int array, where the result is saved.
 * @return indicator whether errors occurred.
 */
int parse_edge(char *edge, int *vertices);

/**
 * @brief Calculates number of edges of an fb arc set.
 * @details Receives an feedback arc sets, calculates
 * the number of edges and returns the result.
 * @param The feedback arc set.
 * @return Number of edges.
 */
int calculate_edges(fb_arc_set *set);

/**
 * @brief Calculates a permutation of vertices.
 * @details Takes in a number of vertices, calculates a permutation and
 * saves the result into vertices.
 * @param Pointer to an int array, where the result is saved.
 * @param Number of vertices.
 */
void permutate(int *vertices, int vertex_count);

