/**
 * @file graph.c
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Implementation of the graph module.
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "graph.h"

/**
 * @brief buffer size macro for space for edges
 * 
 */
#define BUFFER_SIZE 128

void print_edges(graph* src) {
    for (uint32_t i = 0; i < src->num_of_edges; i++) {
        if ((src->edges[i].from != UINT32_MAX) && (src->edges[i].to != UINT32_MAX))
        printf("%d-%d ", src->edges[i].from, src->edges[i].to);
    }
    printf("\n");
}

/**
 * @brief Determines whether a is before b in set
 * @details It iterates through the set, both a and b must be present once.
 * 
 * @param set containing at least a and b
 * @param size size of set
 * @param a 
 * @param b 
 * @return int 1 if a is before b, 0 otherwise
 */
static int is_before(uint32_t* set, uint32_t size, uint32_t a, uint32_t b) {
    int pos_a, pos_b;
    for (int i = 0; i < size; i++) {
        if (set[i] == a) {
            pos_a = i;
        } else if (set[i] == b) {
            pos_b = i;
        }
    }
    return pos_a < pos_b;
}

/**
 * @brief Determines whether edges is already in graph.
 * @details Checks every edges of the specified graph for the specified edge.
 * 
 * @param dst graph
 * @param subject edge to be found in graph
 * @return int 1 if present, otherwise 0
 */
static int contains_edge(graph* dst, edge* subject) {
    edge* dst_edges = dst->edges;
    for (int i = 0; i < dst->num_of_edges; i++) {
        if (dst_edges[i].from == subject->from && dst_edges[i].to == subject->to) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Swaps two variables
 * @details swaps variables using a temporary variable.
 * 
 * @param a pointer to a
 * @param b pointer to b
 */
static void swap(uint32_t* a, uint32_t* b) {
    size_t tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

static void shuffle(uint32_t* coll, uint32_t size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        if (i != j) {
            swap(&(coll[i]), &(coll[j]));
        }
    }
}


/**
 * @brief Returns true if the specified variable is in the collection.
 * @details It iterates through the collection looking for target.
 * 
 * @param coll collection of integers
 * @param size size of collection
 * @param target to be found 
 * @return int 1 if found, otherwise 0
 */
static int contains(uint32_t* coll, uint32_t size, uint32_t target) {
    for (int i = 0; i < size; i++) {
        if (coll[i] == target) {
            return 1;
        }
    }
    return 0;    
}

/**
 * @brief This function permutates the vertices of a graph.
 * @details Iterates through all edges and collects the vertices of the graph.
 * Finally, the algorithm shuffles the vertices.
 * 
 * @param src graph containing edges
 * @param vertices pointer to location vertices are saved to
 * @return int number of vertices
 */
static int permutate_vertices(graph* src, uint32_t** vertices) {
    size_t vertices_size = 0;
    edge curr_edge;
    //get all vertices of the graph
    for (int i = 0; i < src->num_of_edges; i++) {
        curr_edge = src->edges[i];
        if (contains(*vertices, vertices_size, curr_edge.from) == 0) {
            (*vertices)[vertices_size] = curr_edge.from;
            vertices_size++; 
        }
        if (contains(*vertices, vertices_size, curr_edge.to) == 0) {
            (*vertices)[vertices_size] = curr_edge.to;
            vertices_size++;
        }
    }
    shuffle(*vertices, vertices_size);
    return vertices_size;
}

graph* add_edge(graph* dst, edge* edge, int unsafe) {
    if (dst->num_of_edges * sizeof(edge) >= dst->buffer_size) {
        dst->edges = realloc(dst->edges, dst->buffer_size + BUFFER_SIZE);
        dst->buffer_size += BUFFER_SIZE;
        if (dst->edges == NULL) {
            return NULL;
        }
    }
    //check whether graph already has such an edge
    if (contains_edge(dst, edge) == 1 && !unsafe) {
        errno = EINVAL;
        return NULL;
    }
    (dst->edges[dst->num_of_edges]) = *edge;
    dst->num_of_edges++;
    return dst;
}

graph* find_feeback_arc_set(graph* src, graph* dst) {
    edge* src_edges = src->edges;
    //graph has at most edge * 2 vertices
    uint32_t *permutated_set = malloc(sizeof(uint32_t) * src->num_of_edges * 2);
    if (permutated_set == NULL) {
        return NULL;
    }
    uint32_t size = permutate_vertices(src, &permutated_set);

    if (permutated_set == NULL) {
        return NULL;
    }
    //select all edges (u,v) for which u > v in the ordering
    for (int i = 0; i < src->num_of_edges; i++) {
        if (is_before(permutated_set, size, src_edges[i].to, src_edges[i].from) == 1) {
            if (add_edge(dst, &(src_edges[i]), 1) == NULL) {
                return NULL;
            }
        }
    }    
    free(permutated_set);
    return dst;
}
