/**
 * @file generator.c
 * @author Georges Kalmar 9803393
 * @date 11.11.2021
 *
 * @brief Provides some functions for the program.
 * 
 * This module includes the functions that are used for setting the seed for the random numbers,
 * exit the program correctly and calculating the Colors for the nodes and edges to be removed.
 **/
 
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

#include "generator.h"

/** 
 * @brief Function that exits the program
 * @details This function is called if an error occured due to an invalid input of a client, in this case the program cannot continue the tasks properly.
 * Therefore it prints the name of the program accompanied with the valid form inputs should be given and exits the program with EXIT_FAILURE
 * @param myprog is used to give the function the program names Argument using argv[0]
 **/
void abortProg(char* myprog){
	fprintf(stderr,"Usage: %s EDGE1... \nEDGE: a-b  a,b are int and a<b, e.g. 0-1 0-3 2-3 \n", myprog);
	exit(EXIT_FAILURE);
}
/** 
 * @brief Function that sets the seed for the random numbers
 * @details This function is only called once to set the seed according to a microsecond time stamp and should therefore ensure that different 
 * random numbers are created each time the program generator is started.
 **/
void setRandom(void){
	struct timeval t;
	if(gettimeofday(&t,NULL) == -1){
		fprintf(stderr,"Error: gettimeofday failed: %s!\n",strerror(errno));
	}
	srand(t.tv_usec);
}
/** 
 * @brief Function that fills the node Color Array with randomly picked colors
 * @details This function creates the different colors for the nodes using random numbers. The position of the array represents the number of
 * the node, the entry at this position can be 0,1 or 2 for the three colors.
 * @param size_nodeCol_arr is the size of the Node Color Array 
 * @param nodeCol_arr is the array where all the colors are stored
 **/
void colorNodeArr(int size_nodeCol_arr, int* nodeCol_arr){
	for(int i = 0; i < size_nodeCol_arr; i++){
		nodeCol_arr[i] = rand()%3;
	}
}
/** 
 * @brief Function that calculate the edges that have to be removed
 * @details This function is used to check whether the two Nodes linked by an edge have the same color or not by simply comparing their numbers
 * that are stored in the nodeCol_arr. If they are equal the edge is stored in the edge_rm_arr. When a maximum of 8 edges are stored the
 * function stops since this graph is not considered to be a good choice for minimum edges to be removed. 
 * @param edge_arr Array that contains the info of all edges in the graph
 * @param amount_edges gives the number of the edges(size of the first dimension of the edge_arr)
 * @param nodeCol_arr is the array where all the colors are stored
 * @param edge_rm_arr is an array where the edges to be removed are stored
 * @param size_edge_rm_arr this variable is used to store the info on how many edges have to be removed
 **/
void removeEdges(int edge_arr[][2],int amount_edges,int* nodeCol_arr,twoNodes_t edge_rm_arr[],int*size_edge_rm_arr){
	int no1;
	int no2;
	*size_edge_rm_arr=0;
	for(int i = 0; i< amount_edges;i++){
		no1 = edge_arr[i][0];
		no2 = edge_arr[i][1];
		if(nodeCol_arr[no1] == nodeCol_arr[no2]){
			(edge_rm_arr[*size_edge_rm_arr]).n1 = no1;
			(edge_rm_arr[*size_edge_rm_arr]).n2 = no2;
			*size_edge_rm_arr +=1;
			if(*size_edge_rm_arr >=8){return;}
		}
	}
}