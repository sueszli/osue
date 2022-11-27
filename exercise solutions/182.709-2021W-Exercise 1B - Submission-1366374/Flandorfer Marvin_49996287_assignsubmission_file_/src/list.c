/**
 * List module
 * @file list.c
 * @author Marvin Flandorfer, 52004069
 * @date 01.11.2021
 * 
 * @brief Implementation of the list module.
 */

#include <limits.h>
#include "list.h"
#include "misc.h"

edge_t *create_generator_edge_list(char *argv[], int pos_args){
    edge_t *head = (edge_t*) malloc(sizeof(edge_t));        /**< Head of the new list*/
    if(head == NULL){
        error_message("malloc");
        return NULL;
    }
    edge_t *cur = head;                                     /**< Current node in the list*/
    for(int i = 0; i < pos_args; i++){
        char *edge = argv[i+1];                             /**< Edge from argv*/
        long node_nr = strtol(edge, &edge, 10);             /**< Number of the node (can be start and end node)*/
        if(node_nr == LONG_MIN || node_nr == LONG_MAX){
            error_message("strtol");
            free_all_list_nodes(head);
            return NULL;
        }
        cur->from = (int) node_nr;
        edge++;
        node_nr = strtol(edge, &edge, 10);
        if(node_nr == LONG_MIN || node_nr == LONG_MAX){
            error_message("strtol");
            free_all_list_nodes(head);
            return NULL;
        }
        cur->to = (int) node_nr;
        if(i < pos_args-1){
            cur->next = (edge_t*) malloc(sizeof(edge_t));
            if(cur->next == NULL){
                error_message("malloc");
                free_all_list_nodes(head);
                return NULL;
            }
            cur = cur->next;
        }
    }
    cur->next = NULL;
    return head;
}

int push(edge_t *head, int from, int to){
    edge_t *cur = head;                                         /**< Current node of the list*/
    while(cur->next != NULL){
        cur = cur->next;
    }
    cur->next = (edge_t*) malloc(sizeof(edge_t));
    if(cur->next == NULL){
        error_message("malloc");
        return -1;
    }
    cur->next->from = from;
    cur->next->to = to;
    cur->next->next = NULL;
    return 0;
}

edge_t *init_head(int from, int to){
    edge_t *head = (edge_t*) malloc(sizeof(edge_t));            /**< Head of the new list*/
    if(head == NULL){
        error_message("malloc");
        return NULL;
    }
    head->from = from;
    head->to = to;
    head->next = NULL;
    return head;
}

void free_all_list_nodes(edge_t *head){
    edge_t *cur = head;                                        /**< Current node of the list*/
    while(cur != NULL){
        edge_t *next = cur->next;
        free(cur);
        cur = next;
    }
}

int get_number_of_nodes(edge_t *head){
    edge_t* cur = head;                             /**< Current node of the list*/
    int max = -1;                                   /**< Greates node identifier*/
    while(cur != NULL){
        int from = cur->from;
        int to = cur->to;
        if(from > max){max = from;}
        if(to > max){max = to;}
        cur = cur->next;
    }
    return max;
}