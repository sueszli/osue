/**
 * Algorithm module
 * @file algorithm.h
 * @author Marvin Flandorfer, 52004069
 * @date 30.10.2021
 * 
 * @brief This module is responsible for generating feedback arc sets.
 * @details Through random permutation and comparisons a random feedback arc set will be generated.
 * Only feedback arc sets with less or equal elements than 8 will be generated.
 * If the feedback arc set includes 0 elements, it only contains a special element filled with -1.
 */

#ifndef ALGORITHM_H
#define ALGORITHM_H

/**
 * Function for getting a feedback arc set
 * @brief This function generates a feedback arc set and returns it.
 * @details At first it generates a random permutation using the Fisher-Yates-Shuffle.
 * After that it selects the edges according to the Monte-Carlo-Algorithm.
 * 
 * @param head Head of the edge list
 * @param nodes Array of integers containing all nodes
 * @param size_of_nodes Size of the nodes array
 * @return Returns the head of a feedback arc set list on success. Returns NULL on failure.
 */
edge_t *get_feedback_arc_set(edge_t *head, int nodes[], int size_of_nodes);

#endif