#include <stdio.h>
#include <string.h>
#include "graph.h"

char* getColor(int c) {
    switch(c) {
        case 0: return "Red";
        case 1: return "Green";
        case 2: return "Blue";
        default: return NULL;
    }
}

void printGraph(Graph g) {
    printf("Edges: ");
    for(int i=0;i<g.edgeCount;i++) {
        printf("%d-%d ",g.edges[i].v1,g.edges[i].v2);
    }
    printf("\nVertices: ");
    for(int i=0;i<g.vertexCount;i++) {
        printf("%d: %s, ",g.vertices[i].index, getColor(g.vertices[i].color));
    }
    printf("\n");
}

bool hasSameVertexColor(Graph g, Edge e) {
    if(g.vertices[e.v1].color == g.vertices[e.v2].color)
        return true;
    return false;
}