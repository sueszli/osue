/**
 * @file datatypes.h
 * @author Lukas Fink 11911069
 * @date 10.11.2021
 *  
 * @brief datatypes Module
 *
 * contains datatypes required for the generator and supervisor
 **/

#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define MAX_EDGES (8)

/** Edge struct. 
 * @brief Contains an edge with direction start to end
 */
struct Edge
{
    int start;
    int end;
};

/** FeedbackArc struct. 
 * @brief Contains an Array of MAX_EDGES+1 Edges
 * @details the array has size MAX_EDGES+1 because to store the null-edge at the end
 */
struct FeedbackArc
{
    struct Edge feedback[MAX_EDGES+1];
};

struct Edge * constructEdge(char *arr, char *programName);
void showEdges (struct Edge *edgeArr);
struct Edge * removeDuplicateEdges(struct Edge *edgeArr, char *programName, int eSize);
int *makeVerticeList(struct Edge *edgeArr);
void showVertices (int *vertices);
int getSize(int *vertices);
int getSizeEdges (struct Edge *edges);
int getIndex (int *vertices, int entry);

#endif