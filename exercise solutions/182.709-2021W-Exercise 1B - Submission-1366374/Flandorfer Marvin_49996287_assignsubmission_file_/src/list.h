/**
 * List module
 * @file list.h
 * @author Marvin Flandorfer, 52004069
 * @date 01.11.2021
 * 
 * @brief This module is responsible for the list data structure.
 * @details All necessary functions for using the list as well as the struct for the list are provided here.
 * The struct is in the form of an edge in the graph and also contains a pointer to the next edge.
 */

#ifndef LIST_H
#define LIST_H

/**
 * Graph edge struct
 * @brief Struct for the graph edge used in the list data structure.
 */
typedef struct graph_edge{
    int from;                       /**< Number of the node where the edge originates*/
    int to;                         /**< Number of the node where the edge ends*/
    struct graph_edge *next;        /**< Pointer to the next edge*/
}edge_t;

/**
 * Function for creating an edge list for generators
 * @brief Create an edge list for generators.
 * @details All arguments passed to the main function of the program consist of edges (except for the first one).
 * Due to that the edge list for the whole graph can be constructed in one function call.
 * This function also handles convertion of the strings to integers that can be stored in the struct.
 * Memory allocation will also be done here.
 * 
 * @param argv Argument vector used for obtaining all edges
 * @param pos_args Number of positional arguments used for obtaining the number of edges
 * @return Returns a pointer to the head of the list on success. Returns NULL on failure.
 */
edge_t *create_generator_edge_list(char *argv[], int pos_args);

/**
 * Push function
 * @brief This function adds an edge at the end of the list.
 * @details Iterates through all nodes until the end of the list is reached.
 * Edge will be added at the end.
 * The memory for the new edge will be allocated here.
 * 
 * @param head Pointer to the head of the edge list
 * @param from Number of the node where the new edge originates
 * @param to Number of the node where new edge ends
 * @return Returns 0 on success. Returns -1 on failure.
 */
int push(edge_t *head, int from, int to);

/**
 * Function for initializing a new list
 * @brief Initializes head of a new list
 * @details Initializes the first node in the new list.
 * The memory for the head will be allocated here.
 * 
 * @param from Number of node where the head originates
 * @param to Number of node where the head ends
 * @return Returns a pointer to the head of the new list on success. Returns NULL on failure.
 */
edge_t *init_head(int from, int to);

/**
 * Funtion for freeing a list
 * @brief This function frees an entire list.
 * @details The function iterates through all nodes and frees them one by one.
 * 
 * @param head Head of the edge list that needs to be freed
 */
void free_all_list_nodes(edge_t *head);

/**
 * Function for getting the highest node number
 * @brief This function returns the highest occuring node number.
 * @details Iterates through all edges to find the highest occuring node number identifier.
 * 
 * @param head Head of the edge list.
 * @return Returns the highest occuring node number (=identifier).
 */
int get_number_of_nodes(edge_t *head);

#endif