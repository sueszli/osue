/**
 * @file circ_buf.c
 * @author Leonhard Ender (12027408)
 * @date 14.11.2021
 * @brief Functions for using the circular buffer.
 *
 * Contains the functions buf_push and buf_pop which
 * push an item onto or pop it off the buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include "circ_buf.h"

/**
 * @brief Push a feedback arc onto the buffer.
 * @param buf Pointer to the buffer.
 * @param item The feedback arc.
 * @return 0 if succesful, -1 if out of space.
 */
int buf_push(circ_buf *buf, feedback_arc item){

    int next = (buf->head +1)%BUFSIZE;
    
    if (next == buf->tail)
        return -1; // out of space
    
    buf->items[buf->head] = item;
    buf->head = next;

    return 0;
}

/**
 * @brief Pop a feedback arc off the buffer.
 * @param buf Pointer to the buffer.
 * @param item Pointer to which the feedback arc is written.
 * @return 0 if succesful, -1 if buffer is empty.
 */
int buf_pop(circ_buf *buf, feedback_arc *item){

    if (buf->head == buf->tail)
        return -1; // buffer empty

    *item = buf->items[buf->tail];
    buf->tail = (buf->tail+1)%BUFSIZE;

    return 0;
}
