/**
 * @file datastructs.h
 * @author Branislav Balvan <e12023159@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Header file with common structs and macros for fb arc set.
 * 
 * This header file contains common macros and structs used in both
 * generator and supervisor program modules of the feedback arc set
 * problem.
 **/
//RESOURCES NAMES
#define SEM_1 "/12023159_sem_1"
#define SEM_2 "/12023159_sem_2"
#define SEM_3 "/12023159_sem_3"
#define SHM_NAME "/12023159_shm"
//MAX EDGES ALLOWED FOR SOLUTION
#define MAXEDGES 16
//SIZE OF CIRCULAR BUFFER
#define BUFFERSIZE 24

/** @brief Struct for graph edge data. */
typedef struct{
int start, end;
} edge_t;

/** @brief Struct for solution data, containing edges and size. */
typedef struct{
    int ecnt;
    edge_t edges[MAXEDGES];
} solution_t;
