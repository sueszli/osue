/**
 * @file circ_buf.h
 * @author Leonhard Ender (12027408)
 * @date 14.11.2021
 * @brief Contains declarations of buffer functions, structs for 
 * the circular buffer, the buffer and the buffer size.
 */

#define BUFSIZE 32

typedef struct {
    int node1;
    int node2;
} edge;

typedef struct {
    edge edges[8];
    int edge_count;
} feedback_arc;

typedef struct {
    feedback_arc items[32];
    int head; // first free slot
    int tail; // last taken slot
    int shutdown; // 1 indicates supervisor has shut down
} circ_buf;

int buf_push(circ_buf *buf, feedback_arc item);
int buf_pop(circ_buf *buf, feedback_arc *item);
