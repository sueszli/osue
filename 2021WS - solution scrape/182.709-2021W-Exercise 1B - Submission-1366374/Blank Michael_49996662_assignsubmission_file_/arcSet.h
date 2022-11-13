/**
 * @file arcSet.h
 * @author Michael Blank 11909459 <e11909459@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief Provides macros and structs for feedback arc set problem.
 * 
 * @details Contains the names of the semaphore and shared memory files as well as
 * constants that are used by both the generator and supervisor.
 * It also defines the struct for the shared memory and the circular buffer.
 **/

#ifndef COLORING_H
#define COLORING_H

#define FREE_SEM "/11909459_free_sem"
#define USED_SEM "/11909459_used_sem"
#define WRITE_SEM "/11909459_write_sem"
#define SHMNAME "/11909459"

#define MAX_EDGES 8
#define BUFFER_SIZE 50

struct edge {
    int u;
    int v;
};

struct bufEntry {
    int count;
    struct edge edges[MAX_EDGES];
};

struct myshm {
    unsigned int state;
    int wr_pos;
    int rd_pos;
    struct bufEntry buf[BUFFER_SIZE];
};

#endif