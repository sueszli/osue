/**
 * @file GraphUtils.c
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief this module implements functions for managing graphs
 *
 **/

#include <stdlib.h>
#include <string.h>

#include "ErrorHandling.h"
#include "GraphUtils.h"

/**
 * @brief given a permutation of the nodes of a graph a feedback arc set is obtained: all edges (u,v) are selected for which u > v (v comes for u in the permutation) in case the size of the arc set
 * exceeds 8, -1 is returned otherwise 0. The solution is written to the edges location
 */
int fbAlgorithm(struct edge *edges, int max, int adMat[max+1][max+1], int *nodes) {
    int setCount = 0;
    for(int i = 0; i < max; i++) {
        for(int j = i+1; j <= max; j++) {
            int start = nodes[j];
            int end = nodes[i];
            
            if(adMat[start][end] == 1) {
                if(setCount >= 8) {
                    return -1;
                }
                struct edge e = {start,end};
                edges[setCount] = e;
                setCount++;
            }
        }
    }
    return 0;
}

/**
 * @brief given a c-string, an edge is parsed by using strtol and awaiting the following format: "1-0". otherwise a parse error is written to stderr
 * besides that the maximum node number has to be evaluated in order to know the size of the nodes set
 */
struct edge parseEdge(char* input, int* max, char* progName) {
    char delimiter[] = "-";
    char *ptr = NULL;
    unsigned start = 0;
    unsigned end = 0;
    
    ptr = strtok(input,delimiter);
    if(ptr != NULL) {
        start = strtol(ptr, NULL, 10);
    } else {
        failedWithError(progName, "failed with parse error", 1);
    }
    ptr = strtok(NULL, delimiter);
    if(ptr != NULL) {
        end = strtol(ptr, NULL, 10);
    } else {
        failedWithError(progName, "failed with parse error", 1);
    }
    if(start > *max) {
        *max = start;
    }
    if(end > *max) {
        *max = end;
    }
    struct edge e = {start, end};
    return e;
}
