/**
 * @file graph.c
 * @author Gernot Hahn 01304618
 * @date 14.11.2021
 *
 * @brief Implementation of the graph module.
 *
 * @details This module provides implementations of useful graph
 * functions. It contains a function to parse an edge, one to
 * calculate the number of edges of a fb arc set and another one
 * to calculate a random permutation of a set of vertices.
 */

#include "graph.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/**
 * @brief This function pares an edge.
 * @details This function takes a string representation of an edge in the
 * form of "v1-v2", where v1 is the outgoing vertex and v2 the incoming
 * vertex, and saves the vertices into the specified array.
 * @param String representation of the edge.
 * @param Pointer to an int array, where the result is saved.
 * @return 0 if successful, -1 otherwise and sets errno = EINVAL if
 * error occurred.
 */
int parse_edge(char *edge, int *vertices) {
	char *token, *end;

	if (edge[0] == '-' || edge[strlen(edge) - 1] == '-') {
		errno = EINVAL;
		return -1;
	}

	if ((token = strtok(edge, "-")) == NULL) {
		errno = EINVAL;
		return -1;
	}

	vertices[0] = strtol(token, &end, 10);
	if ((vertices [0] == 0 && errno == EINVAL) || *end != '\0')  {
		errno = EINVAL;
		return -1;
	}

	if ((token = strtok(NULL, "-")) == NULL) {
		errno = EINVAL;
		return -1;
	}

	vertices[1] = strtol(token, &end, 10);
	if ((vertices [1] == 0 && errno == EINVAL) || *end != '\0')  {
		errno = EINVAL;
		return -1;
	}

	// check for trailing characters after second vertice
	if ((token = strtok(NULL, "-")) != NULL) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}	

/**
 * @brief Calculates number of edges of an fb arc set.
 * @details Receives an feedback arc sets, calculates
 * the number of edges and returns the result. This
 * counts all edges until a vortex is -1.
 * @param The feedback arc set.
 * @return Number of edges.
 */
int calculate_edges(fb_arc_set *set) {
	int edges = 0;

	for (int i = 0; i < MAX_SET && set->edges[i].v1 != -1; i++) {
		edges++;
	}

	return edges;
}

/**
 * @brief Calculates a permutation of vertices.
 * @details Initializes an int array of size vertex_count and
 * calculates a random permutation of that array based on the
 * Fisher-Yates shuffle.
 * @param Pointer to an int array, where the result is saved.
 * @param Number of vertices.
 */
void permutate(int *vertices, int vertex_count) {
	for (int i = 0; i < vertex_count; i++) {
		vertices[i] = i;	
	}
	// Fisher-Yates shuffle
	for (int i = 0; i < vertex_count - 1; i++) {
		int j = (rand() % (vertex_count - i)) + i;
		int k = vertices[i];
		vertices[i] = vertices[j];
		vertices[j] = k;
	}
}

