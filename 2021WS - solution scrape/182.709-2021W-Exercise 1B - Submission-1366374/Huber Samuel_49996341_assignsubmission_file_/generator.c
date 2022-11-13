/**
 * module name: generator.c
 * @author      Huber Samuel 11905181
 * @brief       generator generates randomized 3-colorable versions of a graph
 * @details     it reads in one or more edges symbolising a graph
 *              and approximates with the help of randomization
 *              close to minimal sets of edges to make the graph 3-colorable
 * @date        31.10.2021
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>

#include "circular_buffer.h"

/**
 * structure representing a graph node
 * @param id: unique number of the saved node
 * @param color: number representing a color given to node
 */
typedef struct {
    int id;
    int color;
} node_t;

node_t *nodes;  // list of all nodes read from input edges
int nodesSize;  // amount of unique nodes read

edge_t *edges;  // list of all edges read from input
int edgesSize;  // amount of unique edges read

edge_t *toDelete;   // list of edges to be deleted in generated solution
int deleteSize;     //amount of edges to be deleted in generated solution

char *prog; // name of the called program

/**
 * @brief prints out usage message
 * @details printed message advises the user, on how to call program properly \n program exits afterwards\n
 *          global variables used: 'prog'
 */
static void usage(void);

/**
 * @brief prints out usage message for edges
 * @details printed message advises the user, on how to input edges properly \n program exits afterwards\n
 *          global variables used: 'prog'
 */
static void usage_edge(void);

/**
 * @brief informs user about occurred error
 * @details prints message of occurred error\n
 *          global variables used: 'prog'
 * @param msg: msg regarding occurred error
 */
static void errorHandling(char *msg);

/**
 * @brief freeing up not needed resources
 * @details used node & edge structures are being freed up, before exiting\n
 *          global variables used:\n
 *          'nodes', 'edges', 'toDelete'
 */
static void freeResources(void);

/**
 * @brief adding node to nodes list
 * @details node_t structure object with given id is added to 'nodes' list,\n
 *          if no node in list already has same id\n
 *          global variables used: 'nodes'
 * @param id: unique number of the node to be added
 */
static void addNode(int id);

/**
 * @brief adding edge to 'edges'
 * @details adds given edge_t structure to list of edges,\n
 *          if given combination of nodes in the edge is not already contained
 *          global variables used:\n
 *          'edges', 'edgesSize'
 * @param edge: structure containing two nodes that should be added
 */
static void addToEdges(edge_t edge);

/**
 * @brief adding edge to 'toDelete'
 * @details adds given edge_t structure to list of edges,\n
 *          if given combination of nodes in the edge is not already contained
 *          global variables used:\n
 *          'toDelete', 'deleteSize'
 * @param edge: structure containing two nodes that should be added
 */
static void addToDelete(edge_t edge);

/**
 * @brief returns color of node
 * @details gets the saved color of a node of the list 'nodes' with the given id\n
 *          global variables used:\n
 *          'nodes', 'nodesSize'
 * @param id unique number of the node of which the color should be returned
 * @return number representing the color of the searched for node
 */
static int getColor(int id);

/**
 * @brief generates randomized 3-colorable solution of a graph
 * @details gives nodes in 'nodes' random color (as number)\n
 *          and picks edges with both nodes having the same color and\n
 *          adding it to 'toDelete' until remaining graph is 3-colorable\n
 *          global variables used:\n
 *          'nodes', 'nodesSize', 'edges', 'edgesSize', 'toDelete', deleteSize
 */
static void generateSolution(void);

static void usage(void) {
    fprintf(stderr, " [%s] USAGE: %s EDGE1...\n", prog,prog);
    exit(EXIT_FAILURE);
}

static void usage_edge(void) {
    fprintf(stderr, "[%s] USAGE: STARTPOINT-ENDPOINT\n",prog);
    exit(EXIT_FAILURE);
}

static void errorHandling(char *msg){
    fprintf(stderr, "[%s] ERROR: %s\n",prog,msg);
    freeResources();
    closeBuffer();
    exit(EXIT_FAILURE);
}

static void freeResources(void) {

    if (nodes != NULL) {
        free(nodes);
        nodes = NULL;
    }
    if (edges != NULL) {
        free(edges);
        edges = NULL;
    }
    if (toDelete != NULL) {
        free(toDelete);
        toDelete = NULL;
    }
}

static void addNode(int id) {
    if (nodes == NULL) {
        nodes = realloc(nodes, (++nodesSize) * sizeof(node_t));
        if(nodes == NULL){
            errorHandling("reallocation of nodes failed");
        }
        nodes->id = id;
    } else {
        for (int i = 0; i < nodesSize; ++i) {
            if (id == nodes[i].id) {
                return;
            }
        }
        nodes = realloc(nodes, (++nodesSize) * sizeof(node_t));
        if(nodes == NULL){
            errorHandling("reallocation of nodes failed");
        }
        nodes[nodesSize - 1].id = id;
    }
}

static void addToEdges(edge_t edge){
    if(edges == NULL){
        edges = realloc(edges,(++edgesSize) * sizeof (edge_t));
        if(edges == NULL){
            errorHandling("reallocation of edges failed");
        }
        edges->from= edge.from;
        edges->to = edge.to;
    } else {
        for (int i = 0; i < edgesSize; ++i) {
            if((edge.from == edges[i].from && edge.to == edges[i].to) || (edge.from == edges[i].to && edge.to == edges[i].from)){
                return;
            }
        }
        edges = realloc(edges, (++edgesSize) * sizeof (edge_t));
        if(edges == NULL){
            errorHandling("reallocation of edges failed");
        }
        edges[edgesSize-1].from = edge.from;
        edges[edgesSize-1].to = edge.to;
    }
}

static void addToDelete(edge_t edge){
    if(toDelete == NULL){
        toDelete = realloc(toDelete,(++deleteSize) * sizeof (edge_t));
        if(toDelete == NULL){
            errorHandling("reallocation of edges failed");
        }
        toDelete->from= edge.from;
        toDelete->to = edge.to;
    } else {
        for (int i = 0; i < deleteSize; ++i) {
            if((edge.from == toDelete[i].from && edge.to == toDelete[i].to) || (edge.from == toDelete[i].to && edge.to == toDelete[i].from)){
                return;
            }
        }
        toDelete = realloc(toDelete, (++deleteSize) * sizeof (edge_t));
        if(toDelete == NULL){
            errorHandling("reallocation of toDelete failed");
        }
        toDelete[deleteSize-1].from = edge.from;
        toDelete[deleteSize-1].to = edge.to;
    }
}

static int getColor(int id){
    for (int i = 0; i < nodesSize; ++i) {
        if(nodes[i].id == id){
            return nodes[i].color;
        }
    }
    errorHandling("no node with given id found, while getting color");
    return -1;
}

static void generateSolution(void){
    toDelete = NULL;
    deleteSize = 0;
    srand(time(0));
    for (int i = 0; i < nodesSize; ++i) {
        nodes[i].color = (rand() % 3);
    }
    for (int i = 0; i < edgesSize; ++i) {
        if(getColor(edges[i].from) == getColor(edges[i].to)){
            addToDelete(edges[i]);
        }
    }
}

/**
 * @brief main method of program
 * @details handles program call and all its arguments,\n
 *          fills node and edge structures,\n
 *          repeatedly generates possible solutions\n
 *          global variables used:\n
 *          'nodes', 'nodesSize', 'edges', 'edgesSize', 'toDelete', deleteSize
 * @param argc: amount of given arguments
 * @param argv: list of given arguments
 * @return if program run successful or with errors
 */
int main(int argc, char *argv[]) {

    nodes = NULL;
    nodesSize = 0;
    edges = NULL;
    edgesSize = 0;
    toDelete = NULL;
    deleteSize = 0;

    prog = argv[0];

    // option handling
    int opt;
    while ((opt = getopt(argc, argv, ":")) != -1) {
        switch (opt) {
            case '?':
                fprintf(stderr, "[%s] ERROR: Wrong use of arguments!\n", prog);
                usage();
                break;
            default:        //should not be reachable -> defensive programming
                assert(0);
        }
    }

    // if argc == 1, only name of program was given
    if (argc - optind < 2) {
        fprintf(stderr, "[%s] ERROR: At least two input graphs needed!\n", prog);
        usage();
    }

    for (int i = 1; i < argc; ++i) {

        edge_t currentEdge;

        if (sscanf(argv[i], "%d-%d", &currentEdge.from, &currentEdge.to) != 2) {
            fprintf(stderr, "[%s] ERROR: Wrong format of EDGE\n", prog);
            usage_edge();
        }

        addNode(currentEdge.from);
        addNode(currentEdge.to);
        addToEdges(currentEdge);
    }

    openBuffer();

    int acyclic = 0;
    while(!isTerminating()){
        generateSolution();
        if(deleteSize < MAX_EDGE_COUNT && deleteSize != edgesSize && !acyclic){
            if(deleteSize == 0){
                acyclic = 1;
            }
            writeBuffer(toDelete, deleteSize);
        }
        free(toDelete);
    }

    freeResources();

    closeBuffer();

    exit(EXIT_SUCCESS);
}
