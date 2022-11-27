#ifndef GRAPH_H
#define GRAPH_H
#define MAX_SOLUTION_SIZE 8

typedef struct{
    int from;
    int to;
} edge;

typedef struct{
    int deletedCount;
    edge deletedEdges[MAX_SOLUTION_SIZE];
} coloringSol;

#endif