/**
* @file generator.c
* @author Laurenz Gaisch <e11808218@student.tuwien.ac.at>
* @date 14.11.2021
*
* @brief Generates solutions to find a minimal feedback arc set
* @details Take all nodes, reshuffle them randomly. Then removes edges (u,v) where u > v in the reshuffled list.
 * Saves the removed edges in a circular buffer via shared memory.
*/


#include <stdio.h>
#include <memory.h>
#include <limits.h>
#include "circular-buffer.h"

/**
 * Converts a string array to long
 * @param arg
 * @param endptr
 * @param i
 * @param name
 * @return long-value of the input
 */
int convertToLong(char *arg, char **endptr, int i,char* name) {
    int val = strtol(arg,endptr,10);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        fprintf(stderr, "[%s] Converting to long failed.\n",name);
        exit(EXIT_FAILURE);
    }
    if (endptr == &arg) {
        fprintf(stderr, "[%s] No digits were found.\n",name);
        exit(EXIT_FAILURE);
    }
    return val;
}

/**
 * Finds the array position of node in the uniqueNodes array
 * @param node
 * @param uniqueNodes
 * @param uniqueCounter
 * @return 0: node1 <= node2 | 1: node1 > node2
 */
int secondNodeBeforeFirst (int node1, int node2, int uniqueNodes[],int uniqueCounter) {
    for(int j = 0; j<uniqueCounter;j++){
        if(node1 == uniqueNodes[j]) {
            return 0;
        }
        if(node2 == uniqueNodes[j]) {
            return 1;
        }
    }
    fprintf(stderr,"Something went wrong getting node position in array.");
    exit(EXIT_FAILURE);
}

/**
 * Reshuffles the uniqueNodes randomly
 * @param uniqueNodes
 * @param size of uniqueNodes
 */
void reshuffleNodes(int *uniqueNodes,int size) {
    time_t t;
    srand((unsigned) time(&t));

    for (int i = 0; i < size; i++)
    {
        int j = rand() % size;
        int t = uniqueNodes[j];
        uniqueNodes[j] = uniqueNodes[i];
        uniqueNodes[i] = t;
    }
}

/**
 * If u > v of the vertices (u,v), remove the edge. If there were less nodes removed than before, write it in the buffer.
 * @param firstNodesOfEdge first node of each edge
 * @param secondNodesOfEdge second node of each edge
 * @param edgeSize amount of edges
 * @param uniqueNodes
 * @param nodeSize of uniqueNodes
 * @param min number of removed edges
 * @return the amount of edges removed
 */
int findFeedbackArcSet(int *firstNodesOfEdge, int *secondNodesOfEdge, int edgeSize, int *uniqueNodes, int nodeSize, int min) {
    const int SIZE = 8;
    int fbsetFirst[SIZE];
    int fbsetSecond[SIZE];
    int fbsetCounter = 0;

    for (int i = 0; i < edgeSize -1; i++) {
        if(secondNodeBeforeFirst(firstNodesOfEdge[i],secondNodesOfEdge[i],uniqueNodes,nodeSize) == 1) {
            if(fbsetCounter >= SIZE) {
                return min;
            }
            fbsetFirst[fbsetCounter] = firstNodesOfEdge[i];
            fbsetSecond[fbsetCounter++] = secondNodesOfEdge[i];
        }
    }
    if(fbsetCounter < min) {
        writeBuffer(fbsetFirst,fbsetSecond,fbsetCounter);
        return fbsetCounter;
    }
    return min;
}

/**
 * Startpoint of the generator class
 * @brief Finds all unique nodes by first splitting the input into all nodes and then filtering out duplicate nodes
 * @details Prints the amount of edges that need to be removed for the graph to be acyclic. This is the generator class
 * that finds the solutions in order to store them in the shared memory.
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[]) {
    //Opens existing sm
    openMemory();

    int firstNodes[argc-1],secondNodes[argc-1], allNodes[(argc-1)*2];

    char *endptr;

    //splitting the input and converting it to the correct format
    for(int i = 1;i<argc;i++) {
        //get the first part of the argument in the form of Node-Node | (e.g. 1-2) into "1"
        char *arg = strtok(argv[i],"-");
        long val;
        //attempt to get the long int value of the arg
        val = strtol(arg,&endptr,10);
        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
            || (errno != 0 && val == 0)) {
            fprintf(stderr, "[%s] Input could not be read. Edges have to be displayed like: x-y y-z z-x\n",argv[0]);
            perror("strtol");
            exit(EXIT_FAILURE);
        }
        firstNodes[i-1]=convertToLong(arg,&endptr,10,argv[0]);
        allNodes[(i-1)*2] = firstNodes[i-1];

        //get the second part of the arg
        arg = strtok(NULL, "-");
        secondNodes[i-1]=convertToLong(arg,&endptr,10,argv[0]);
        allNodes[(i-1)*2+1] = secondNodes[i-1];
    }

    int *uniqueNodes;

    if(!(uniqueNodes = (int*) malloc((argc-1)*2 * sizeof(int))))
    {
        fprintf(stderr, "%s: Something went wrong allocating memory.",argv[0]);
        exit(EXIT_FAILURE);
    }

    //gets an array of all unique nodes in allnodes
    int unique,uniqueCounter = 0;
    for(int i = 0; i<(argc-1)*2;i++) {
        unique=1;
        for(int j = 0; j<i;j++){
            if(allNodes[j] == allNodes[i]){
                unique = 0;
                break;
            }
        }
        if(unique==1){
            uniqueNodes[uniqueCounter++] = allNodes[i];
        }
    }
    uniqueNodes = realloc(uniqueNodes,uniqueCounter * sizeof(int));

    int min = 8;
    while (min > 0 && stillRunning() < 2) {
        reshuffleNodes(uniqueNodes,uniqueCounter);
        min = findFeedbackArcSet(firstNodes,secondNodes,argc,uniqueNodes,uniqueCounter,min);
    }
    free(uniqueNodes);
    disposeRes();
    return EXIT_SUCCESS;
}