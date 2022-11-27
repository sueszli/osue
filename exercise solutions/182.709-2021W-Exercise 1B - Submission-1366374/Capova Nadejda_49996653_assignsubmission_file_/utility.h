
/**
 * @file   utility.h
 * @author Nadejda Capova (11923550)
 * @date   26.10.2021
 *
 * @brief Provides types and initialization shared by generator and supervisor.
 **/
#ifndef UTILITY_H
#define UTILITY_H

#define MAX_EDGES 8  //the maximum edges for a suitable solution//
#define MAX_DATA (4096/sizeof(arrayEdge_t))  //size of the shared memory
#define SHM_NAME "/11923550myshm" //name of shared memory
#define FILL "/11923550sem_fill"     //semaphore tracking the free space
#define EMPTY "/11923550sem_empty"    //semaphore tracking the used space
#define MUTEX "/11923550sem_mutex"    //mutually exclusive access to the write end of the circular buffer


//Type definitions
//edge represents one node with the both end points of one edge and its color
typedef struct edge
{
    int a;
    int colorA;
    int b;
    int colorB;
    int toRemove;
} edgeNode;

//arrayEdge has an array from edgeNodes. It contains as well the count of nodes in the array.
typedef struct arrayEdge
{
    edgeNode node[MAX_EDGES];
    int countEdges;
} arrayEdge_t;

//Define structure of the shared memory. It has arrayEdge_t with fixed size and variable which indicates when the mapping should stop
struct myshm
{
    arrayEdge_t result[MAX_DATA];
    unsigned int ifStop; //vaiable to stop
};

#endif
