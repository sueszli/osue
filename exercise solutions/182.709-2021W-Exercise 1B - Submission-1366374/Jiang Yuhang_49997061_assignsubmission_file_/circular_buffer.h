/**
 * @file circular_buffer.h
 * @author Yuhang Jiang <e12019854@student.tuwien.ac.at>
 * @brief Structs and read/write methods for the circular buffer.
 * @date 30.10.2021
 */

#ifndef AUFGABE1B_CIRCULAR_BUFFER_H
#define AUFGABE1B_CIRCULAR_BUFFER_H

#define CIRCULAR_BUFFER_SIZE 10
#define MAX_EDGES 16

/**
 * @brief Saves the colorability, the solution length and the nodes. Nodes of the same index have an edge.
 */
struct edges {
    int colorable;              /**< 0 = not fully colorable, 1 = colorable without removing any edge */
    unsigned int solution_len;  /**< Solution length */
    int node1[MAX_EDGES];       /**< First node of the edges */
    int node2[MAX_EDGES];       /**< Second node of the edges */
};

/**
 * @brief The structure of the circular buffer.
 * @details The maximum size of the buffer is defined in CIRCULAR_BUFFER_SIZE
 */
struct circular_buffer {
    unsigned int wr_pos;
    unsigned int rd_pos;
    struct edges buffer[CIRCULAR_BUFFER_SIZE];
};

/**
 * @brief Writes the value to the circular buffer.
 * @param cbuf The ciruclar buffer
 * @param val The value
 */
void cbuf_write(struct circular_buffer *cbuf, struct edges *val);

/**
 * @brief Reades a value from the circular buffer.
 * @param cbuf The circular buffer
 * @return The value read from the buffer
 */
struct edges cbuf_read(struct circular_buffer *cbuf);

#endif //AUFGABE1B_CIRCULAR_BUFFER_H
