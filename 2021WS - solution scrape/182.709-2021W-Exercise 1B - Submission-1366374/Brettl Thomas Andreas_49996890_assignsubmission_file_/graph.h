#ifndef GRAPH_H
#define GRAPH_H

#define SHM_NAME "/shm11902122"
#define MAX_ENTRIES (50)
#define MAX_EDGES (20)
#define SEM_FREE "/sem11902122free"
#define SEM_USED "/sem11902122used"
#define SEM_WRITE "/sem11902122write"

#include <stdbool.h>

enum Coloring {red,green,blue};

typedef struct {
    int index, color;
} Vertex;

typedef struct {
    int v1,v2;
} Edge;

typedef struct {
    int edgeCount,vertexCount;
    Edge* edges;
    Vertex* vertices;
} Graph;

char* getColor(int c);
void printGraph(Graph g);
bool hasSameVertexColor(Graph g, Edge e);

#endif