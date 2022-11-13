/** 
*@file: 	graphs.h
*@author:	Christopher Graßl (11816883)
*@date:		14.11.2021
*
*@brief:	This header contains types and declarations of functions
*			for the generator to work on Graphs
*@details:	implementation of the functions in graphs.c
*/


#ifndef GRAPHS_H
#define GRAPHS_H

#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

//Edge of a Graph
typedef struct edge{
	unsigned long firstNode; 	//Größe der möglichen Graphen begrenzt
	unsigned long secondNode;	//durch Datentyp des Node Index
} edge_t;

//Node of a Graph
typedef struct node{
	unsigned long index;
	int color;
}node_t;

//Solution for the 3color Problem
typedef struct solution{
	int value;			//number of edges of the solution
	edge_t edges[8];
}solution_t;


/** 
*add node function
*@brief: 	Function to add a Node to a node array representing 
			nodes of a graph	
*@details:	Takes the Index of a node and a existing array of nodes
			as input and adds the new Node to the array if there's no
			node with the same index already
*@param:	nodeInd		Index of the new node
*@param:	node_arr	pointer to a node array
*@param:	nodeCount	number of nodes in the node array
*@return:	returns the new Number of nodes in the array
*/
unsigned long addNode(unsigned long nodeInd, node_t **node_arr, unsigned long nodeCount);


/** 
*solve function
*@brief: 	Function to find a solution to the 3-Color problem for
			a given Graph
*@details:	takes a Graph in form of pointers to a node array and an edge array
			as well as the Number of nodes and number of edges. These are then used
			to find a solution for the 3-Color Problem by coloring the nodes randomly
			and removing the necessary edges to make it valid. The edges that need
			to be removed are then put in the edge array of a solution
*@param:	node_arr		array of nodes
*@param:	nodeCount		number of nodes in the array
*@param:	edge_arr		array of edges
*@param:	edgeCount		number of edges in the array
*@param:	sol_arr			pointer to the edge array of a solution
*@return:	returns the value of the solution on success, -1 if solution was too large
*/
int solve(node_t *node_arr, unsigned long nodeCount,
			edge_t *edge_arr, unsigned long edgeCount, edge_t *sol_arr);



#endif