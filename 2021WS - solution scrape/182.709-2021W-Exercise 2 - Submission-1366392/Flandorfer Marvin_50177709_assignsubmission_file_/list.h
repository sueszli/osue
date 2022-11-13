/**
 * @file list.h
 * @author Marvin Flandorfer, 52004069
 * @date 23.11.2021
 * 
 * @brief List module.
 * @details This module contains the list struct and all functions for using the list.
 */

#ifndef LIST_H
#define LIST_H

/**
 * List struct
 * @brief Struct for list nodes containg points (x- and y- coordinates).
 */
typedef struct coordinate{
    float x;                            /**< x-coordinate of the point*/
    float y;                            /**< y-coordinate of the point*/
    struct coordinate *next;            /**< next node in the linked list*/
}coord_t;

/**
 * @brief Function for initiating the list.
 * @details This functions sets the head of the list and returns the pointer to the head.
 * 
 * @param x x-coordinate
 * @param y y-coordinate
 * @return Returns a pointer to the head of the list if successful. Returns null if an error occured.
 */
coord_t *init(float x, float y);

/**
 * @brief Function for pushing a new node to the list.
 * @details This function creates a new list node and adds it to the end of list referenced by head.
 * 
 * @param head Pointer pointing to the head of the list
 * @param x x-coordinate
 * @param y y-coordinate
 * @return Returns 0 on success and -1 if an error occured.
 */
int push(coord_t *head, float x, float y);

/**
 * @brief Function for freeing the list.
 * @details This function frees all nodes in the list.
 * 
 * @param head Pointer pointing to the head of the list.
 */
void free_list(coord_t *head);

/**
 * @brief Function for getting the size of the list.
 * @details This function iterates through the entire list and returns the size of the list.
 * 
 * @param head Pointer pointing to the head of the list.
 * @return Returns the size of the list as an integer.
 */
int size_list(coord_t *head);

#endif