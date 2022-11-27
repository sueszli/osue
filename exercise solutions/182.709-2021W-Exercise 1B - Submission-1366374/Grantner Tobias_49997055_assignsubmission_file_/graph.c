#include "graph.h"
#include <stdio.h>

edge_t create_edge(int x, int y) {
    edge_t edge = { x, y };
    return edge;
}

graph_t create_graph(const edge_t * edges, size_t count) {
    if(count > GRAPH_MAX_EDGE_COUNT) {
        return (graph_t) { .count = -1 };
    }

    graph_t graph = { .count = count };

    int i;
    for(i = 0; i < count; i++) {
        graph.edges[i] = edges[i];
    }
    return graph;
}

/**
 * @brief Inserts a node into a sorted array of nodes, but only if it doesnt exist yet
 * 
 * @param nodes The sorted array of nodes where the node should be inserted in, should be at least count + 1 big
 * @param count The amount of nodes already in the nodes array
 * @param node The node to be inserted
 * @return size_t The count of nodes in the nodes array after the insertion
 */
size_t insert_node(int * nodes, size_t count, int node) {
    int start = 0;

    while(start < count && nodes[start] < node) start++;

    if(nodes[start] == node) {
        return count;
    }

    int end = count;

    while(end > start) {
        nodes[end] = nodes[end - 1];
        end--;
    }

    nodes[start] = node;
    return count + 1;
}

size_t get_nodes(int * destination, const edge_t * edges, size_t edgeCount) {
    size_t nodeCount = 0;
    int i;
    for(i = 0; i < edgeCount; i++) {
        nodeCount = insert_node(destination, nodeCount, edges[i].x);
        nodeCount = insert_node(destination, nodeCount, edges[i].y);
    }
    return nodeCount;
}

int print_edges(FILE * output, const edge_t * edges, size_t count) {
    int i;
    for(i = 0; i < count; i++) {
        if(i != 0) {
            if(fprintf(output, " ") < 0) return -1;
        }
        if(fprintf(output, "%d-%d", edges[i].x, edges[i].y) < 0) return -1;
    }

    return 0;
}
