/*
 * @author Vladyslav Hnatiuk(01613669)
 * @brief  Datastructures for multi-process arc set algorithm
 * @date  13-11-2021
 * */

#ifndef UE1_B_DATASTRUCTURES_H
#define UE1_B_DATASTRUCTURES_H

#define MAX_EDGES   20
#define BUFFER_SIZE 20

/*
 * @brief  One graph edge
 * */
struct edge {
    int node1;
    int node2;
};

struct edge_list {
    int edge_count;
    struct edge edges[MAX_EDGES];
};

struct list_of_edge_lists {
    int end;
    struct edge_list lists[BUFFER_SIZE];
};

#endif //UE1_B_DATASTRUCTURES_H
