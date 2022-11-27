/**
 * @file graph.h
 * @author Jakob Guttmann 11810289 <e11810289@student.tuwien.ac.at>
 * @brief this module contains methods for working with graph structures
 * @details defines a struct for storing directed edges and methods for graphs, especially useful for fb_arc computation
 * permutate nodes
 * search node in shuffled array
 * perform monte carlo algorithm to get feedback arc set
 * prints an arcset to stdout
 * 
 * @date 11.11.2021
 * 
 * 
 */
#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <time.h>

/**
 * @brief type for storing directed edges
 * src is beginning of edge
 * dest is end of edge
 * 
 */
typedef struct edg
{
    int src, dest;
} Edge;

/**
 * @brief gets array of vertices and shuffles this vector
 * @details performs Fisher-Yates algorithm to shuffle the vector
 * 
 * @param nodes array of vertices which is changed in this method
 * @param length how many nodes are in the array
 */
void permutate(int *nodes, int length);

/**
 * @brief search given node in the vertices array and returns position in this array
 * 
 * @param nodes shuffeld nodes array
 * @param node node to be searched in array
 * @param length length of nodes array
 * @return int returns index of node in array or -1 if node is not in the array
 */
int search(int *nodes, int node, int length);

/**
 * @brief prints given feedback arcset to stdout
 * 
 * @param arcset the edges which are a feedback arcset
 * @param size size of the feedback arcset (how many edges)
 * @param caller which program this function called
 */
void printArcset(Edge *arcset, int size, char *caller);

/**
 * @brief performs monte carlo algorithm with given nodes and edges and returns a feedback arcset which is smaller than actual minimum
 * if method finds an arcset greater than actualmin then it returns -1, otherwise the size of the arcset
 * @details all edges (u,v) where the vertice u is in the permutation after v forms an feedback arc set
 * this property is used in this algorithm
 * goes through all edges and takes the edges which have this property
 * if already more edges are selected then actual minimum, method returns -1 and needs newly shuffled array 
 * 
 * @param numEdges number of edges in the graph
 * @param numNodes number of node in the graph
 * @param actualMin actual minimum of arcset
 * @param permutNodes nodes in arbitrary order
 * @param edges edges in the graph
 * @param arcset arcset which is returned (but must have at least size of eight)
 * @return int number of edges in arcset, if actual arcset is smaller than actual min then -1 is returned
 */
int feedback_arc_set(int numEdges, int numNodes, int actualMin, int *permutNodes, Edge *edges, Edge *arcset);

#endif