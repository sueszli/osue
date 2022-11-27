/** @file ispalindrom.h
* @author Larissa Artlieb 
* @date 13.11.2021
* @brief header file for supervisor
* @details 	header file for supervisor assignment 1B*/


#define SHM_NAME "/feedback"
#define FREESPACESEM_NAME "/freeSpaceSem"
#define USEDSPACESEM_NAME "/usedSpaceSem"
#define WRITEPOSSEM_NAME "/writePosSem"
#define MAX_EDGES (8)
#define NODES_PER_EDGE (2)
#define MAX_DATA_SETS (40)

/// circular buffer struct for shared memory
struct circular_buffer {
    volatile int stop;
    int candidateSets[MAX_DATA_SETS][MAX_EDGES][NODES_PER_EDGE];
    int numberOfEdges[MAX_DATA_SETS];
    int wr_pos;
};

void readFromBuffer(char* progname);
