/**
 * @file generator.h
 * @author Georges Kalmar 9803393
 * @date 11.11.2021
 *
 * @brief Provides some defines, typedef struct definitions and functions for the program.
 * 
 * This header includes the definitions for the Shared Memory and semaphores. The size
 * of the Shared Memory is defined by the ARRSIZE. Also the typedefs for the struct are added.
 * Furthermore some important functions are included for setting the seed for the random numbers,
 * exit the program correctly and calculating the Colors for the nodes and edges to be removed.
 **/
 
#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h> 	
#include <fcntl.h>
#include <semaphore.h>  	
#include <errno.h>



#define SHM_NAME "/g_shm_3334col"
#define SEM1 "/g_sem1_3334col"
#define SEM2 "/g_sem2_3334col"
#define SEM3 "/g_sem3_3334col"
#define ARRSIZE (50)

typedef struct twoNodes{
	int n1;
	int n2;
}twoNodes_t;

typedef struct twoNodes_arr{
	twoNodes_t arr[8];
	int size;
}twoNodes_arr_t;

/** 
 * @brief Function that exits the program
 * @details This function is called if an error occured due to an invalid input of a client, in this case the program cannot continue the tasks properly.
 * Therefore it prints the name of the program accompanied with the valid form inputs should be given and exits the program with EXIT_FAILURE
 * @param myprog is used to give the function the program names Argument using argv[0]
 **/
void abortProg(char* myprog);
/** 
 * @brief Function that sets the seed for the random numbers
 * @details This function is only called once to ensure that different random numbers are created each time the program generator is started.
 * So each generator process that runs in parallel gets a different starting number.
 **/
void setRandom(void);
/** 
 * @brief Function that fills the node Color Array with randomly picked colors
 * @details This function creates the different colors for the nodes using random numbers and stores them in an array 
 * @param size_nodeCol_arr is the size of the Node Color Array 
 * @param nodeCol_arr is the array where all the colors are stored
 **/
void colorNodeArr(int size_nodeCol_arr, int* nodeCol_arr);
/** 
 * @brief Function that calculate the edges that have to be removed
 * @details This function is used to check whether the two Nodes linked by an edge have the same color or not. If they do have this edge is added 
 * to an array that finally contains the info of all the edges that have to be removed. The amount of edges to be removed is also stored. 
 * @param edge_arr Array that contains the info of all edges in the graph
 * @param amount_edges gives the number of the edges(size of the first dimension of the edge_arr)
 * @param nodeCol_arr is the array where all the colors are stored
 * @param edge_rm_arr is an array where the edges to be removed are stored
 * @param size_edge_rm_arr this variable is used to store the info on how many edges have to be removed
 **/
void removeEdges(int edge_arr[][2],int amount_edges,int* nodeCol_arr,twoNodes_t edge_rm_arr[],int*size_edge_rm_arr);

#endif