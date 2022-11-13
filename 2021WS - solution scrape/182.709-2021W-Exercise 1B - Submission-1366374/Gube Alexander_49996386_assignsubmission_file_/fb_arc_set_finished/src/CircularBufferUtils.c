/**
 * @file CircularBufferUtils.c
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief implements util functions which facilitate the usage of a CircularBuffer
 *
 **/


#include <stdlib.h>
#include <stdio.h>
#include "CircularBufferUtils.h"

/**
 * @details the solution has to be of maximum size 8, in case it is less than 8, free-edges are represented by (0-0)
 */
void print(struct fbArc solution) {
    for(int i = 0; i < 8; i++) {
        struct edge e = solution.edges[i];
        if(e.start == 0 && e.end == 0) {
            continue;
        }
        printf("%d-%d ", e.start, e.end);
    }
    printf("\n");
}

/**
 * @details first the size of the solution is determined and based on it, the new best solution is evaluated
 */
void validate(struct fbArc solution, struct fbArc *bestSolution, int *bestCount) {
    int solutionCount = 0;
    for(int i = 0; i < 8; i++) {
        struct edge e = solution.edges[i];
        if(e.start == 0 && e.end == 0) {
            continue;
        }
        solutionCount++;
    }
    if(solutionCount < *bestCount) {
        *bestCount = solutionCount;
        *bestSolution = solution;
        print(solution);
    }
}
