/** 
*@file: 	graphs.c
*@author:	Christopher Graßl (11816883)
*@date:		14.11.2021
*
*@brief:	This file contains implementations of functions
*			for the generator to work on Graphs
*@details:	associated header file: graphs.h
*/


#include "graphs.h"



//returns the color of the Node with the given index "ind"
/**
*@brief:	gets the Color of a node
*@details:	The Function takes a node array with number of nodes
*			and an index as input and then returns the integer
*			value representing the color of the node with the
*			given index
*@param:	node_arr		array of nodes
*@param:	nodeCount		number of nodes in the array
*@param:	ind				index of the node to get the color from
*@return:	returns the int value representing the color of the node
*/
static int getNodeColor(node_t *node_arr, unsigned long nodeCount, unsigned long ind){
	int ret = -1;
	for(int l=0;l<nodeCount;l++){
		if(node_arr[l].index == ind){
			ret = node_arr[l].color;
		}
	}
	return ret;
}



/**
*@brief:	adds a Node to a node array representing the nodes of a graph
*@details:	elaborate documentation in graphs.h
*/
unsigned long addNode(unsigned long nodeInd, node_t **node_arr, unsigned long nodeCount){
	//check if node already exists
	for(unsigned long j=0;j<nodeCount;j++){
		if((*node_arr)[j].index == nodeInd){
			return nodeCount;
		}
	}	

	//if not, then the Node is added and the nodeCount increased by 1

	node_t *new_arr = realloc(*node_arr, (nodeCount+1) * sizeof(node_t));
	if(new_arr == NULL){
		fprintf(stderr,"generator: failed to reallocate memory in addNode()");
		exit(EXIT_FAILURE);
	}
	*node_arr = new_arr;

	(*node_arr)[nodeCount].index = nodeInd;
	(*node_arr)[nodeCount].color = 0; //represents "no Color"
	nodeCount += 1;

	return nodeCount;
}


/**
*@brief:	solves the 3-Color Problem for a Graph and returns a solution
*@details:	elaborate documentation in graphs.h
*/
int solve(node_t *node_arr, unsigned long nodeCount,
			edge_t *edge_arr, unsigned long edgeCount, edge_t *sol_arr){

	int m=0;
	
	//set seed for rand()
	srand((unsigned)time(NULL));

	//Assign random colors to the Nodes of the Graph
	for(int k=0; k<nodeCount; k++){
		node_arr[k].color = ((rand() % 3)+1);
	}

	//get the color of both nodes of an edge
	for(int k=0; k<edgeCount;k++){
		int color1 = getNodeColor(node_arr,nodeCount,edge_arr[k].firstNode);
		if(color1 == -1){
			fprintf(stderr, "generator: failed to get the color of a node");
			exit(EXIT_FAILURE);
		}
		int color2 = getNodeColor(node_arr,nodeCount,edge_arr[k].secondNode);
		if(color2 == -1){
			fprintf(stderr, "generator: failed to get the color of a node");
			exit(EXIT_FAILURE);
		}

		if(color1 == color2){			//if nodes have the same color put edge in the solution
			if(m>7){
				return -1;
			}
			sol_arr[m] = edge_arr[k];
			m += 1;
		}

	}


	return m;
}
