/**
 * @file graph.c
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 09.11.2021
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"

node_t *find_node_by_id(graph_t *graph, long id) {
    for (int i = 0; i < graph->node_count; ++i) {
        if (graph->nodes[i].id == id) {
            return &graph->nodes[i];
        }
    }
    return NULL;
}

void color_randomly(graph_t *graph) {
    for (int i = 0; i < graph->node_count; ++i) {
        graph->nodes[i].color = rand() % 3;
    }
}

long get_deletion_edges(graph_t *graph, long *buffer) {
    long size = 0;
    for (int i = 0, pos = 0; i < graph->edge_count; ++i) {
        if (graph->edges[i].node1->color == graph->edges[i].node2->color) {
            buffer[pos++] = graph->edges[i].node1->id;
            buffer[pos++] = graph->edges[i].node2->id;
            size++;
        }
    }
    return size;
}

graph_t *copy(graph_t *graph) {
    graph_t *new = malloc(sizeof(graph_t));
    new->node_count = graph->node_count;
    new->edge_count = graph->edge_count;

    /** Assign old nodes */
    new->nodes = malloc(sizeof(node_t) * graph->node_count);
    memcpy(new->nodes, graph->nodes, sizeof(node_t) * graph->node_count);

    /** Assign edges */
    new->edges = malloc(sizeof(edge_t) * graph->edge_count);
    for (int i = 0; i < graph->edge_count; ++i) {
        node_t *node1 = find_node_by_id(new, graph->edges[i].node1->id);
        node_t *node2 = find_node_by_id(new, graph->edges[i].node2->id);
        if (node1 == NULL || node2 == NULL) {
            fprintf(stderr, "COPY ERROR: NODE NOT FOUND");
            exit(EXIT_FAILURE);
        }
        new->edges[i].node1 = node1;
        new->edges[i].node2 = node2;
    }

    return new;
}


void delete_graph(graph_t *graph) {
    free(graph->edges);
    free(graph->nodes);
    free(graph);
}
