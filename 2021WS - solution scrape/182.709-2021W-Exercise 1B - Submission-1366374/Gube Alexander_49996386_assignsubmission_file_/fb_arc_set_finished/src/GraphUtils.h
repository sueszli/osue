/**
 * @file GraphUtils.h
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief this module provides functions to graphs and provides a struct to represent edges in a graph
 *
 **/

/** edges
 * @brief represents an graph edge by non-negative start- and end-node
 */
struct edge {
     unsigned int start;
     unsigned int end;
};

/**
 * @brief This function findes a solution feedback arc and writes it to edges
 * @param edges the solution is written to this location
 * @param max amount of nodes in a graph
 * @param adMat adjacent matrix of the graph which the problem is applied to
 * @param nodes permutation of the graph's nodes
 * @returns 0 on success, -1 otherwise
 * */
int fbAlgorithm(struct edge *edges, int max, int adMat[max+1][max+1], int *nodes);

/**
 * @brief handles the input pos arguments and converts each argument to an edge
 * @param input c-string representation of an edge
 * @param max amount of nodes in a graph
 * @param progName name of program
 * @returns an edge 
 * */
struct edge parseEdge(char* input, int* max, char* progName);
