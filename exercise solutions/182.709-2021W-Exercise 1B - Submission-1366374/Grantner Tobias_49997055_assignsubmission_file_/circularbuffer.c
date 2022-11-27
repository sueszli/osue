#include "circularbuffer.h"
#include "graph.h"
#include <stdio.h>

/**
 * @brief Increases an index by 1, jumping to the beginning if it surpasses the end
 * 
 * @param index Pointer to the index which should be increased
 */
void increase(size_t * index) {
    *index = (*index + 1) % GRAPH_MAX_EDGE_COUNT;
}

void cb_push(circularbuffer_t * cb, graph_t graph) {
    size_t index = cb->write_head;
    increase(&cb->write_head);
    cb->data[index] = graph;
}

graph_t cb_pop(circularbuffer_t * cb) {
    size_t index = cb->read_head;
    increase(&cb->read_head);
    return cb->data[index];
}
