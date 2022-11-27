/**
 * @file graph.c
 * @author Jannis Adamek (11809490)
 * @date 2021-11-14
 *
 * @brief Implementation of graph.h
 **/

#include "graph.h"

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Graph *malloc_graph(size_t number_of_edges) {
  Graph *graph = (Graph *)malloc(sizeof(Graph));
  graph->edge_arr = (Edge *)calloc(number_of_edges, sizeof(Edge));
  graph->next_edge_index = 0;
  graph->edge_color_arr = NULL;
  graph->highest_vertex_num = 0;
  graph->number_of_edges = number_of_edges;
  return graph;
}

void malloc_edge_color_arr(Graph *graph) {
  graph->edge_color_arr = (int *)calloc(graph->highest_vertex_num, sizeof(int));
}

void add_edge_to_graph(Graph *graph, const char *edge_represent) {
  // It is a precondition of this function that graph can accommodate at least
  // one more edge.
  assert(graph->number_of_edges > graph->next_edge_index);

  Edge edge;
  sscanf(edge_represent, "%zu-%zu", &(edge.v1), &(edge.v2));
  if (edge.v1 > edge.v2) {
    size_t tmp = edge.v1;
    edge.v1 = edge.v2;
    edge.v2 = tmp;
  }
  graph->edge_arr[graph->next_edge_index].v1 = edge.v1;
  graph->edge_arr[graph->next_edge_index].v2 = edge.v2;
  graph->next_edge_index++;

  if (edge.v2 > graph->highest_vertex_num) {
    graph->highest_vertex_num = edge.v2;
  }
}

void color_graph_randomly(Graph *graph) {
  srand(time(NULL));
  for (int i = 0; i < graph->highest_vertex_num; i++) {
    graph->edge_color_arr[i] = rand() % 3;
  }
}

void get_excess_edges(Graph *graph, EdgeSet *edge_set) {
  // Go through all edges and check whether the color of both vertices is
  // identical. If it is, the edge gets added to the excess_edges set.

  edge_set->next_index = 0;

  for (int i = 0; i < graph->number_of_edges; i++) {
    Edge cur = graph->edge_arr[i];
    if (graph->edge_color_arr[cur.v1] == graph->edge_color_arr[cur.v2]) {
      edge_set->edges[edge_set->next_index] = cur;
      edge_set->next_index++;
    }
    // If it is necessary to remove more than MAX_EDGES_IN_EXCESS_SET edges
    // from the function returns NULL to indicate that this round is not
    // successful.
    if (edge_set->next_index >= MAX_EDGES_IN_EXCESS_SET) {
      memset(edge_set->edges, 0, sizeof(Edge) * MAX_EDGES_IN_EXCESS_SET);
      edge_set->next_index = 0;
      break;
    }
  }
}

void free_graph(Graph *graph) {
  if (graph == NULL) {
    return;
  }
  if (graph->edge_arr != NULL) {
    free(graph->edge_arr);
  }
  if (graph->edge_color_arr != NULL) {
    free(graph->edge_color_arr);
  }
  free(graph);
}

void print_edge_set(const EdgeSet *edge_set) {
  for (int i = 0; i < edge_set->next_index; i++) {
    printf("%zu-%zu ", edge_set->edges[i].v1, edge_set->edges[i].v2);
  }
}
