/**
 * @file generatorfunctions.h
 * @author Atheer Elobadi <e01049225@student.tuwien.ac.at>
 * @date 08.11.2021
 *  
 * @brief 
 */


#ifndef GENERATORFUNCTIONS_H
#define GENERATORFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>
#include "utils.h"

#define USAGE "Usage: %s EDGE1...\n"

typedef struct
{
    edge_t *edge;
    uint8_t size;
    unsigned int top;
} graph_t;

extern int row; 
extern int sizeOfSolution;
extern set_t graph; 

/**
 * @brief Adds an edge, which is represented with a string, to the given graph.
 * 
 * @param[in] edge a string representation of the edge which should be added to the graph. 
 * @param[in,out] graph the graph to which the edges should be added. 
 */
void add_string_edge_to_graph(char *edge, graph_t *graph);

/**
 * @brief   Initialise the given graph.
 * @details The size, top and all the edges will be set to 0. 
 * 
 * @param[in, out] graph the graph which will be initialised. 
 */
void init_graph(graph_t *graph);

/**
 * @brief Print the graph to stdout. 
 * 
 */
void print_graph();

/**
 * @brief Parse the string arguments which represent the edges into integer u
 * 
 * @param[in] edge  the string representation of the the edge which will 
 *                  be parsed. 
 * @param[out] u    the u vertice of the given edge. 
 * @param[out] v    the v vertice of the given edge. 
 */
void parse_vertices(char *edge, uint32_t *u, uint32_t *v);

/**
 * @brief   creates a random peremutation array of a given size 
 *          (representing the vertices)
 * 
 * @param[out] rndP the array which will be randomised.  
 * @param[in] size  the size of the array. 
 */
void random_peremutation(uint32_t *rndP, unsigned int size);

/**
 * @brief Get feedback arc set of a given graph
 * 
 * @param[in] graph the graph for which the feedback arc set should be found
 * @return set_t* a pointer to the resulting feedback arc set. 
 */
set_t *get_feedback_arc_set(graph_t graph);

/**
 * @brief Get the number of the vertices in a given graph
 * 
 * @param[in] graph the graph for which the number of vertices should be found. 
 * @return int      the number of vertices. 
 */
int get_number_of_vertices(graph_t graph);

/**
 * @brief 
 * 
 * @param rndP 
 * @param number_of_vertices 
 * @param edge 
 * @return int 
 */
int compare_order_of_u_and_v(uint32_t rndP[], int number_of_vertices, edge_t edge);

/**
 * @brief print the synopsis to stdin
 * 
 * @param[in] program_name the name of this program (argv[0]).
 */
void usage(char* program_name); 

#endif


