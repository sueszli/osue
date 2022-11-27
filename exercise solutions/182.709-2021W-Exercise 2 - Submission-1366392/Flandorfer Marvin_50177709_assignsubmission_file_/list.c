/**
 * @file list.c
 * @author Marvin Flandorfer, 52004069
 * @date 23.11.2021
 * 
 * @brief Implementation of the list module.
 */

#include <stdlib.h>
#include "list.h"
#include "misc.h"

coord_t *init(float x, float y){
    coord_t *head = malloc(sizeof(coord_t));        /**< Pointer to the head list node*/
    if(head == NULL){
        error_message("malloc");
        return NULL;
    }
    head->next = NULL;
    head->x = x;
    head->y = y;
    return head;
}

int push(coord_t *head, float x, float y){
    coord_t *cur = head;                            /**< Pointer to the current list node*/
    while(cur->next != NULL){
        cur = cur->next;
    }
    cur->next = malloc(sizeof(coord_t));
    if(cur->next == NULL){
        error_message("malloc");
        return -1;
    }
    cur->next->x = x;
    cur->next->y = y;
    cur->next->next = NULL;
    return 0;
}

void free_list(coord_t *head){
    coord_t *cur = head;                            /**< Pointer to the current list node*/
    while(cur != NULL){
        coord_t *next = cur->next;
        free(cur);
        cur = next;
    }
}

int size_list(coord_t *head){
    coord_t *cur = head;                            /**< Pointer to the current list node*/
    int count = 0;
    while(cur != NULL){
        count++;
        cur = cur->next;
    }
    return count;
}