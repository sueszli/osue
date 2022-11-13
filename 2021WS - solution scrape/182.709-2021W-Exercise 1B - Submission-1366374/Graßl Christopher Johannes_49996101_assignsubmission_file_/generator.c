/** 
* @file: 	generator.c
* @author: 	Christopher Graßl (11816883) <e11816883@student.tuwien.ac.at>
* @date: 	14.11.2021
*
*@brief: 	main programm module for the 3-coloring generator.
* 			Opens shared memory and semaphores, creates representation
*			of a graph from its arguments and tries to find a Solution
*			to the 3-Coloring problem for the graph and writes it
*			to the shared memory
*@details: 	the generator opens the shared memory object and the semaphores
*			created by the supervisor and writes solutions to the 3-Coloring problem
*			in it. It takes a set of edges as input which are converted into a better
*			representation of a graph (one node array and one edge array). It then
*			assigns each node a random color and puts all edges that need to be
*			removed to make the graph 3-colorable into a solution array which is then 
*			copied into the shared memory
*/


#include <unistd.h>
#include "graphs.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include "shm.h"


char *progName;		/**< programm name */

/** 
*Usage function
*@brief: 	This function writes usage information to stderr
@details: 	global variables: progName
*/
static void usage(void){
	fprintf(stderr, "Usage: %s EDGE1...\n EXAMPLE: %s 0-1 0-2 0-3 1-2 1-3 2-3\n",progName,progName);
	exit(EXIT_FAILURE);
}

/**
*errorExit function
*@brief: This function writes Information about an occurred error and terminates the programm
*@details: 	The input string is used to print a helpful error message to stderr
*			global variables: progName
*@param: msg: String which is printed to stderr
*/
static void errorExit(char *msg){
	fprintf(stderr, "%s: %s \n", progName, msg);
	fprintf(stderr, "cause: %s \n", strerror(errno));
	exit(EXIT_FAILURE);
}


/** 
*main programm
*@brief: 	entry point to the programm
*@details:	This function takes care of opening resources,
*			converts it's arguments in a representation of 
*			a graph and then tries to find a solution to
*			the 3-Coloring problem for the graph
*@param: 	argc Argument counter
*@param: 	argv Argument vector
*@return:	The programm terminates with EXIT_SUCCESS
*/
int main(int argc, char *argv[]){

	progName = argv[0];
	edge_t *edge_arr = malloc(sizeof(edge_t));
	node_t *node_arr = malloc(sizeof(node_t));
	unsigned long nodeCount = 0;
	unsigned long edgeCount = 0;

	if(argc < 2){
		free(edge_arr);
		free(node_arr);
		usage();
	}

	//read input Graph
	for(int i=1; i<argc; i++){
		char *endptr = NULL;
		char *separatingchar = NULL;

		unsigned long node1 = strtoul(argv[i], &separatingchar, 10);
		//check if the String starts with a number
		if(argv[i] == separatingchar){
			free(edge_arr);
			free(node_arr);
			usage();
		}

		//if the first value was valid *separatingchar now points to the "-" char
		//
		if(*separatingchar != '-'){
			free(edge_arr);
			free(node_arr);
			usage();
		}
		//
		unsigned long node2 = strtoul((separatingchar+1), &endptr, 10);
		//check if second Number is valid
		if(*endptr != '\0'){
			free(edge_arr);
			free(node_arr);
			usage();
		}

		if(node1 != node2){	//if the edge is a loop it is ignored otherwise it is added to the array
			if(i>1){
				edge_t *new_arr = realloc(edge_arr, i * sizeof(edge_t));
				if(new_arr == NULL){
					free(edge_arr);
					free(node_arr);
					errorExit("failed to reallocate edge array");
				}
				edge_arr = new_arr;
			}
			edge_arr[i-1].firstNode = node1;
			edge_arr[i-1].secondNode = node2;

			edgeCount +=1;

			nodeCount = addNode(node1,&node_arr,nodeCount);
			nodeCount = addNode(node2,&node_arr,nodeCount);

		}else{ //if the Edge is a loop we only have to add 1 Node
			nodeCount = addNode(node1,&node_arr,nodeCount);
		}


	}

	//open shared mem
	buffer_t *circBuff = NULL;
	int fileDescriptor = 0;

	if((fileDescriptor = openSHM(&circBuff)) == -1){
		free(edge_arr);
		free(node_arr);
		errorExit("failed to open shared memory");
	}


	//open semaphores
	sem_t *freeSem = NULL;
	if(openSem(&freeSem,"/11816883_freeSem") == -1){
		free(edge_arr);
		free(node_arr);
		errorExit("failed to open 'freeSem' Semaphore");
	}
	sem_t *used = NULL;
	if(openSem(&used,"/11816883_usedSem") == -1){
		free(edge_arr);
		free(node_arr);
		errorExit("failed to open 'used' Semaphore");
	}
	sem_t *excl = NULL;
	if(openSem(&excl,"/11816883_exclSem") == -1){
		free(edge_arr);
		free(node_arr);
		errorExit("failed to open 'excl' Semaphore");
	}


	while(!(circBuff->done)){
		solution_t solution;
		int val = solve(node_arr,nodeCount,edge_arr,edgeCount,solution.edges);
		if(val >= 0){
			//write Solution into Buffer
			solution.value = val;
			if(sem_wait(excl)==-1){
				if(errno == EINTR){
					continue;
				}else{
					free(edge_arr);
					free(node_arr);
					errorExit("Error waiting on 'excl' Semaphore");
				}
			}

			if(sem_wait(freeSem) == -1){
				if(errno == EINTR){
					continue;
				}else{
					free(edge_arr);
					free(node_arr);
					errorExit("Error waiting on 'freeSem' Semaphore");
				}
			}

			circBuff->solutions[circBuff->writePos] = solution;
			circBuff->writePos = ((circBuff->writePos + 1)%10);

			if(sem_post(used)==-1){
				free(edge_arr);
				free(node_arr);
				errorExit("failed to increment 'used' Semaphore");
			}

			if(sem_post(excl) == -1){
				free(edge_arr);
				free(node_arr);
				errorExit("failed to increment 'excl' Semaphore");
			}
		}

	}

	//close shared mem and semaphores
	if(closeSHM(circBuff, false, fileDescriptor) == -1){
		free(edge_arr);
		free(node_arr);
		errorExit("closing shared memory failed");
	}

	//close semaphores
	if(closeSem(freeSem, "/11816883_freeSem",false) == -1){
		free(edge_arr);
		free(node_arr);
		errorExit("closing 'freeSem' Semaphore failed");
	}

	if(closeSem(used, "/11816883_usedSem",false)==-1){
		free(edge_arr);
		free(node_arr);
		errorExit("closing 'used' Semaphore failed");
	}

	if(closeSem(excl, "/11816883_exclSem",false)==-1){
		free(edge_arr);
		free(node_arr);
		errorExit("closing 'excl' Semaphore failed");
	}

	free(edge_arr);
	free(node_arr);
	exit(EXIT_SUCCESS);
}
