/**
 * @file generator.c
 * @author Leopold Fajtak (e0152033@student.tuwien.ac.at)
 * @brief See header
 * @version 0.2
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "generator.h"

/**
 * @brief Name of this program. (argv[0])
 * 
 */
static char* myprog;

/**
 * @brief prints usage message and terminates the process
 * 
 */
static void usage(void);

/**
 * @brief writes error message and terminates the process
 * 
 * @param msgStart Start of the error message
 */
static void handle_error(char* msgStart);

/**
 * @brief Keeps looking for solutions and writes them to shared memory until it is told not to by a varible in the shared memory
 * 
 * @param g Input graph
 */
static void searchForSolutions(graph* g);

/**
 * @brief Generates a random array of size size with entries
 * in {0,1,2}.
 * The memory is dynamiclly allocated and has to be freed!
 * 
 * @param size Size of the arry to be generated
 * @return int* Dynamically allocated array
 */
static int* random3coloring(int size);

int main(int argc, char** argv){
    myprog = argv[0];
    edge* e=NULL;
    graph* g=malloc(sizeof(graph));
    if(g==NULL){
        handle_error("failed to allocate memory");
    }
    memset(g, 0, sizeof(*g));
    g->numEdges=0;

    if(argc<=1){
        usage();
        exit(EXIT_FAILURE);
    }
    for(int i=1; i<argc; i++){
        e=scanEdge(argv[i]);
        if(e==NULL){
            handle_error("Failed scanning edge");
        }
        addEdge(g, *e);
        free(e);
    }
    
    if(open_circ_buf()!=0)
        handle_error("unable to shared memory");
    searchForSolutions(g);
    if(close_circ_buf()!=0)
        handle_error("unable to close shared memory");
}

static void searchForSolutions(graph* g){
    bool exit = false;
    int* coloring = NULL;
    int size = numNodes(g);
    graph* s = malloc(sizeof(graph));
    int bufWriteStatus=0;

    while(exit==false){
        coloring = random3coloring(size);
        initGraph(s);
        for(int i=0; i<g->numEdges; i++){
            if(edgeMakesColoringInvalid(coloring, g->edges[i])){
                addEdge(s, g->edges[i]);
            }
        }
        bufWriteStatus = writeToBuf(s);
        if(bufWriteStatus==-1)
            handle_error("error writing to buffer");
        if(bufWriteStatus==-2)
            exit=true;
    }
    free(coloring);
    free(s);
}

static void usage(void){
    fprintf(stderr, "Usage: %s EDGE1 ...", myprog);
}

static void handle_error(char* msgStart){
    fprintf(stderr, "%s: %s: %s\n", myprog, msgStart, strerror(errno));
    exit(EXIT_FAILURE);
}

static int* random3coloring(int size){
    int* coloring = malloc(size*sizeof(int));
    for(int i=0; i<size; i++){
        coloring[i] = rand() % 3;
    }
    return coloring;
}

