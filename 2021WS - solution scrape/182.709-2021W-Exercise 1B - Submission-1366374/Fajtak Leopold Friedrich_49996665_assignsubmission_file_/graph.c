/**
 * @file graph.c
 * @author Leopold Fajtak (e0152033@student.tuwien.ac.at)
 * @brief see header
 * @version 0.2
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "graph.h"

/**
 * @brief Get the Edge object in graph g on index i
 * 
 * @param g 
 * @param i 
 * @return edge* 
 */
static edge* getEdge(graph* g, int i);

/**
 * @brief print the edge e according to the pattern %d-%d
 * 
 * @param e 
 */
static void printEdge(edge* e);

edge* scanEdge(char* s){
    edge *newEdge = malloc(sizeof(edge));
    memset(newEdge, 0, sizeof(edge));
    
    if(sscanf(s, "%u-%u", &(newEdge->from), &(newEdge->to))==EOF)
        return NULL;
    return newEdge;
}

static edge* getEdge(graph* g, int i){
    assert(g->numEdges>i);
    return (g->edges) + i;
}

void printGraph(graph *g){
    for(int i=0; i<(g->numEdges); i++){
        printEdge(getEdge(g, i));
        printf(" ");
    }
    printf("\n");
}

int addEdge(graph* g, edge e){
    if(g->numEdges>=MAX_EDGES){
        errno = ENOSPC;
        return -1;
    }
    //memcpy is used in datastructure to make sure that everything referenced in shared memory
    //will actually be located in shared memory
    (g->edges)[(g->numEdges)]=e;
    (g->numEdges)++;
    return 0;
}

bool edgeMakesColoringInvalid(int* coloring, edge e){
    return coloring[e.from] == coloring[e.to];
}

int numNodes(graph* g){
    int m =0;
    for(int i=0; i<g->numEdges; i++){
        if(m<g->edges[i].from)
            m=g->edges[i].from;
        if(m<g->edges[i].to)
            m=g->edges[i].to;
    }
    return m;
}

void initGraph(graph* g){
    g->numEdges = 0;
    for(int i=0; i<MAX_EDGES; i++){
        g->edges[i].from=0;
        g->edges[i].to=0;
    }
}

static void printEdge(edge* e){
    printf("%d-%d", e->from, e->to);
}
