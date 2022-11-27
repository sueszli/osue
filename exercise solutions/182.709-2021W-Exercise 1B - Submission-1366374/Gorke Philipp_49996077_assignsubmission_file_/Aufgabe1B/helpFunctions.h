/**
 * @file helpFunctions.h
 * @author Philipp Gorke <e12022511@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief definitions
 * 
 * This help file just contains includes and definitions which are used in both
 * programs
 *
 **/




#ifndef HELPFUNCTIONS_H
#define HELPFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include "helpFunctions.h"
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <signal.h>


#define SEM_1 "/12022511_freespace"
#define SEM_2 "/12022511_usedspace"
#define SEM_3 "/12022511_mutex"
#define SHM_NAME "/12022511_buf"


/**
 * @brief Struct to store an edge.
 * @details end vertex and start vertex as int
 */
typedef struct {
    int v1; 
    int v2;
} edge;


/**
 * @brief a solution of more edges
 * @details struct: which holds maximum 8 edges. It's one soluttion for 
 * 3 color problem. Nmb = number of removed edges
 */
typedef struct {
    edge edges[8];
    int nmb; 
} graph;


/**
 * @brief shared Memory
 * @details Shared Memory as circular Buffer, data = graphs
 * bool kill tells the generator to stop, wr_pos and rd_pos
 * are the positions in which the coresponding programms have 
 * to write or write in the graphs data
 */
typedef struct
{
    graph graphs[900];
    unsigned int wr_pos;
    unsigned int rd_pos;
    bool kill;
} mySharedMem;






#endif