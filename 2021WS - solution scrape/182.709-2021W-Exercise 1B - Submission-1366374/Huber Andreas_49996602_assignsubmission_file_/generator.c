/** 
 * @file generator.c
 * @author Andreas Huber 11809629
 * @date 12.11.2021
 *
 * @brief Generator program to generate possible solutions for the fib_arc_set
 * @details generator reads the input vertices and edges, shuffels them with the Yates_shuffler
 * and permutates the input to possible solutions until termination is called.
 **/

#include "circularBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

typedef struct Edge{
    long u;
    long v;
} edge;

/**
 * @brief format Edge function to format edges correctly for writing into the buffer
 * 
 * @param out the correct output for the circular buffer
 * @param e the edge to be formatted and added to the output
 * @return char[] the formatted output with the added edge e
 */
char* formatEdge(char* out, edge* e){
    int u = e->u;
    int v = e->v;
    strcat(out,u);
    strcat(out,"-");
    strcat(out,v);
    strcat(out," ");
}

/**
 * @brief yates_shuffler for array-permutation
 * 
 * @param vertices int[] of vertices to be shuffled
 * @param size size of the vertices-array
 */
static void shuffle(int* vertices, int size){
    for(int i = size-1; i>=1; i--){
        int j = random() % (i+1);
        int temp = vertices[i];
        vertices[i] = vertices[j];
        vertices[j] = temp;  
    }
}

/**
 * @brief indexOf returns the index of a given vertex in an array
 * 
 * @param vertex vertex thats index is beeing looked up
 * @param array array in which the vertex is present
 * @param size size of the array given
 * @return int index of looked up vertex in the array
 */
int indexOf(const int vertex, const int *array, int size)
{
    while (size--)
    {
        if (array[size] == vertex)
            return size;
    }
}

/**
 * Program entry point.
 * @brief Program starts here. generator function to generate permutations of a given
 * input of edges in a graph until termination is called for. First the circular buffer
 * is opened, 
 * @details 
 *          This function uses the following global variables:
 *          ProgramName: stores the Programname from argv[0]
 *          shmem: shared memory object that represents the circular buffer
 *          edgeCount: number of input edges of the given graph 
 *          edges: array of input edges
 *          numberOfVertices: number of total Vertices in the given graph 
 *          min: number of minimal arc_fib_set (standard 9 as results above are not viable)
 *          solutionEdgeAmmount: counter for ammount of edges a soultion has found
 *          solutionEdgeIndex[]: index of Edges that can be removed to provide a solution
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_FAILURE if opening of circular buffer (generatorOpenCircularBuffer()) failed
 * @return EXIT_FAILURE if input edges are not in the right format (u-v)
 * @return EXIT_FAILURE if a vertex of a found solution is not present in the inputgraph (should never happen)
 * @return EXIT_FAILURE if writing to the circular buffer failed
 * @return EXIT_SUCCESS otherwise
 */
int main(int argc, char* argv[]){

    //open circular buffer
    const char* ProgramName = argv[0];
    sharedMemory* shmem = generatorOpenCircularBuffer();
    if(shmem == NULL){
        fprintf(stderr, "[%s] ERROR: generatorOpenCircularBuffer failed: %s\n", ProgramName, strerror(errno));
        exit(EXIT_FAILURE);    
    }

    int edgeCount = argc-1;
    int edges[edgeCount];
    int numberOfVertices=0;

    //iterate over input edges, save them to edges[], get number of vertices in the graph
    for(int i = 0; i<edgeCount; i++){

        char* v1 = argv[i+1];
        char* endptr = NULL;
        long u = strtol(v1, &endptr, 10);
        if(endptr[0]!='-'){
            fprintf(stderr, "[%s] ERROR: Wrong Vertex delimiter (should be '-'): %s\n", ProgramName, strerror(errno));
            exit(EXIT_FAILURE);   
        }
        edge* e = edges[i];
        e->u = u;

        char* v2 = endptr+1;
        long v = strtol(v2, &endptr, 10);
        if(endptr[0]!=' '){
            fprintf(stderr, "[%s] ERROR: Wrong space between vertices (should be ' '): %s\n", ProgramName, strerror(errno));
            exit(EXIT_FAILURE);   
        }
        e->v = v;
        free(v1);
        free(v2);

        numberOfVertices = MAX(numberOfVertices, u);
        numberOfVertices = MAX(numberOfVertices, v);  
    }

    //set random seed and prepare ouput-array
    srand(getpid()*time(NULL));
    int output[numberOfVertices];
    for(int i=0;i<numberOfVertices;i++){
        output[i]=i;
    }

    //permutate array, check if solution is viable, discard if not
    int min = 9;
    int solutionEdgeAmmount = 0;
    int solutionEdgeIndex[edgeCount];
    while(shmem->terminate != true){

        shuffle(output,numberOfVertices);
        solutionEdgeAmmount = 0;

        for(int i = 0; i<edgeCount;i++){
            //get index of vertex in each edge
            edge* e = edges[i];
            int indexOfU = indexOf(e->u, output, numberOfVertices);
            int indexOfV = indexOf(e->v, output, numberOfVertices);

        if(indexOfU == -1 ){
            fprintf(stderr, "[%s] ERROR: indexOf failed: %s\n", ProgramName, strerror(errno));
            exit(EXIT_FAILURE);   
        }
        if(indexOfV == -1){
            fprintf(stderr, "[%s] ERROR: indexOf failed: %s\n", ProgramName, strerror(errno));
            exit(EXIT_FAILURE);   
        }
        if(indexOfU > indexOfV){
            solutionEdgeIndex[solutionEdgeAmmount] = i;
            solutionEdgeAmmount++;
        }

        }
      

        if(solutionEdgeAmmount<min){
            min = solutionEdgeAmmount;

            char* data;
            for(int i = 0; i<solutionEdgeAmmount;i++){
                edge* e = edges[solutionEdgeIndex[i]];
                formatEdge(data, e);
            }
            if(writeCircularBuffer(shmem, data)==-1){
                fprintf(stderr, "[%s] ERROR: writeCircularBuffer failed: %s\n", ProgramName, strerror(errno));
                exit(EXIT_FAILURE); 
            }
        }
    }
    return(EXIT_SUCCESS);
}