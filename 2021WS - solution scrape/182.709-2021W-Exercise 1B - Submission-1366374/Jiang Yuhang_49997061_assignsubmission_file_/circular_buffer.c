/**
 * @file circular_buffer.c
 * @author Yuhang Jiang <e12019854@student.tuwien.ac.at>
 * @brief Implementation of read and write to a circular buffer.
 * @date 30.10.2021
 */

#include "circular_buffer.h"

/**
 * @brief The program name
 */
extern char *PROG_NAME;

/**
 * @brief Writes the value to the circular buffer.
 * @param cbuf The circular buffer
 * @param val The value
 */
void cbuf_write(struct circular_buffer *cbuf, struct edges *val)
{
    cbuf->buffer[cbuf->wr_pos] = *val;
    cbuf->wr_pos += 1;
    cbuf->wr_pos %= CIRCULAR_BUFFER_SIZE;
}

/**
 * @brief Reades a value from the circular buffer.
 * @param cbuf The circular buffer
 * @return The value read from the buffer
 */
struct edges cbuf_read(struct circular_buffer *cbuf)
{
    struct edges val = cbuf->buffer[cbuf->rd_pos];
    cbuf->rd_pos += 1;
    cbuf->rd_pos %= CIRCULAR_BUFFER_SIZE;
    return val;
}
