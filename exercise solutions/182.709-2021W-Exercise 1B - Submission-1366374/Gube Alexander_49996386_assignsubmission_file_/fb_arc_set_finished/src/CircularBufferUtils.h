/**
 * @file CircularBufferUtils.h
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief this module provides util functions for the circularBuffer and defines the structure of the saved values
 *
 **/

#include "GraphUtils.h"

#define bufferSize 32                   //each entry consists of 8 x 2 int
#define shmName "/12023988circularBuffer"

/** fbArc
 * @brief represents a feedback solution arc with a maximum of 8 edges
 */
struct fbArc {
    struct edge edges[8];
};

/**
 * @brief prints a feedback arc set to stdout in the format (e.g. "1-0 2-3")
 * @param solution solution to be printed
 * */
void print(struct fbArc solution);

/**
 * @brief validate a solution and replace the current best with the new one in case its size is smaller
 * @param solution a new solution which is not yet validated
 * @param bestSolution current best solution (if any)
 * @param besCount size of the current best solution
 * */
void validate(struct fbArc solution, struct fbArc *bestSolution, int *bestCount);

