#include "circularbuffer.h"
#include <time.h>

/**
 * @brief struct to save a directed edge of a directed graph (node1 --> node2)
 * */

/**
 * @brief returns a random number representing a color
 * @details returns a random number representing a color
 * @return returns a random number representing a color
*/ 
int get_random_color();

/**
 * @brief colors nnodes
 * @details assigns a number to each node (number representing a color)
 * @param node_count number of nodes
 * @return returns an array; indeces stand for the nodes, value (int) stand for a color
*/ 
int* color_graph(int node_count);

/**
 * @brief looks for the highest node name
 * @details looks for the highest node name
 * @param graph graph to check
 * @param edge_count number of edges
 * @return highest node name
*/
int get_node_count(edge* graph, int edge_count);

/**
 * @brief generates a graph
 * @details generates a graph with given input
 * @param argc argument counter
 * @param argv arguments
 * @param graph edge array
 * @return true if successful, false otherwise (in case of an error)
 * */
void generate_graph(int argc, char** argv, edge* graph);

/**
 * @brief get result
 * @details calls all needed functions to generate a result
 * @param graph edge array
 * @param node_count number of nodes
 * @param edge_count number of edges
 * @param colored_graph int-array, that stores the color of each node
 * @return returns a (random) result
 * */
result get_result(edge* graph, int node_count, int edge_count, int* colored_graph);