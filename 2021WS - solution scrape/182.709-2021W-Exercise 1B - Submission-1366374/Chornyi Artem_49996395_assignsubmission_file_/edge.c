/**
 * @author Artem Chornyi. 11922295
 * @brief Structures and macros for circular buffer and its properties.
 * @details Length of whole circular buffer is 64*17= 1088 bytes, somewhat 1 KiB
 * @date 9-th November 2021 (09.11.2021)
 * 
 */


#define EDGE
#define MAX_EDGES_PER_ENTRY (8)
#define CIR_BUFFER_LENGTH (64)

typedef unsigned char vertex;
typedef struct
{
    // Direction of an edge, from -> to, from-to
    vertex from,to;
} edge;

typedef struct
{
    edge edges[MAX_EDGES_PER_ENTRY];
    // to keep track of valid edges.
    unsigned char edgesAmount;
} cir_buffer_entry;

typedef struct 
{
    cir_buffer_entry entries[CIR_BUFFER_LENGTH];
} cir_buffer;

