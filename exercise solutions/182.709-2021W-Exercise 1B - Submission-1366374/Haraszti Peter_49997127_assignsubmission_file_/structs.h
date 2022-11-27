/**
 * @file structs.h
 * @author Peter Haraszti <Matriculation Number: 12019844>
 * @date 08.11.2021
 *
 * @brief Structs, Semaphores and Shared Memory used by both generator and supervisor
 * @details In structs.h, the semaphores and common constants of supervisor and generator are initialized.
 * myshm is the shared memory, Edge is self explanatory and Solution is the struct for storing solutions in the cirular buffer
 */

#define BUFFER_LENGTH 30

struct Edge {
    int to;
    int from;
};

struct Solution {
    int numEdges;
    struct Edge edges[8];
};

struct myshm {
    int wr_pos;
    unsigned int stop;
    struct Solution buff[BUFFER_LENGTH];
};

