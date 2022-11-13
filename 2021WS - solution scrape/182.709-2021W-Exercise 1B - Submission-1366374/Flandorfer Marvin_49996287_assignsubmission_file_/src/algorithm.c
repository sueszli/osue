/**
 * Algorithm module
 * @file algorithm.c
 * @author Marvin Flandorfer, 52004069
 * @date 30.10.2021
 * 
 * @brief Implementation of the algorithm module.
 */

#include <time.h>
#include "list.h"
#include "misc.h"

/**
 * Fisher-Yates-Shuffle
 * @brief This function computes a random permutation using the Fisher-Yates-Shuffle.
 * @details The random numbers will be generated through a random number generator that gets seeded beforhand.
 * The seed will be determined through a realtime clock.
 * 
 * @param elements Array of integers that will be shuffled
 * @param size Size of elements in the array
 * @return Returns 0 on success. Returns -1 on failure.
 */
static int fisher_yates_shuffle(int *elements, int size){
    for(int i = size-1; i > 0; i--){
        struct timespec now;                                    /**< Timespec struct for time*/
        int error = clock_gettime(CLOCK_REALTIME, &now);
        if(error < 0){
            error_message("clock_gettime");
            return -1;
        }
        srandom(now.tv_nsec);
        int j = random() % size;
        int a = elements[i];
        elements[i] = elements[j];
        elements[j] = a;
    }
    return 0;
}

/**
 * Function for selecting edges
 * @brief This function selects the edges for the solution
 * @details Selects edges according to the Monte-Carlo-Algorithm.
 * If not edges get selected (graph is acyclic) a list will be created which only contains one node and is filled with -1.
 * In every iteration a comparison between order and edges will be made and if all requirments are met, the edge will be selected and will be added to solution.
 * 
 * @param head_of_all_edges Pointer to the head of the graph edge list
 * @param permutation Permuted array of nodes
 * @param size_of_perm Size of the permuted array
 * @return Returns a pointer to the head of the feedback arc set on success. Returns NULL on failure.
 */
static edge_t *select_edges(edge_t *head_of_all_edges, int permutation[], int size_of_perm){
    int edge_count = 0;                                 /**< Counter that counts the number of edges in the solution*/
    edge_t *head = NULL;                                /**< Pointer that points to the head of the solution*/
    edge_t *cur = head_of_all_edges;                    /**< Pointer that points to the head of the graph edge list*/
    while(cur != NULL){
        if(edge_count > 8){
            free_all_list_nodes(head);
            if((head = init_head(-2,-2)) == NULL){
                return NULL;
            }
            return head;
        }
        for(int i = 0; i < size_of_perm; i++){
            if(cur->from == permutation[i]){break;}
            if(cur->to == permutation[i]){
                if(head == NULL){
                    head = init_head(cur->from, cur->to);
                    if(head == NULL){
                        return NULL;
                    }
                    edge_count++;
                }
                else{
                    int a = push(head, cur->from, cur->to);
                    if(a < 0){
                        free_all_list_nodes(head);
                        return NULL;
                    }
                    edge_count++;
                }
                break;
            }
        }
        cur = cur->next;
    }
    if(head == NULL){
        head = init_head(-1,-1);
        if(head == NULL){
            return NULL;
        }
    }
    return head;
}

edge_t *get_feedback_arc_set(edge_t *head, int nodes[], int size_of_nodes){
    edge_t *selection_head = NULL;                                      /**< Head of list of selected edges*/
    int permutation[size_of_nodes];                                     /**< Permutation of the nodes*/
    for(int i = 0; i < size_of_nodes; i++){
        permutation[i] = nodes[i];
    }
    int a = fisher_yates_shuffle(permutation, size_of_nodes);
    if(a < 0){
        return NULL;
    }
    selection_head = select_edges(head, permutation, size_of_nodes);
    if(selection_head == NULL){
        return NULL;
    }
    return selection_head;
}