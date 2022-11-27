#define FREE_SPACE "/01633034_FREE_SPACE"
#define USED_SPACE "/01633034_USED_SPACE"

#define SHARED_MEM "/01633034_SHARED_MEMORY"
#define WRITE_INDEX "/01633034_WRITE_INDEX"

#define MAX_BUFFER 32
#define MAX_BUFFER_INDEX (MAX_BUFFER - 1)

typedef unsigned int vertex_t;
typedef char *input_edge_t;

typedef struct vertex_data
{
    vertex_t vertex;
    int color;
} vertex_data_t;

typedef struct edge
{
    vertex_t from;
    vertex_t to;
} edge_t;

typedef struct graph_data
{
    edge_t *edges;
    int edge_count;

    vertex_data_t *vertices;
    int vertex_count;
} graph_data_t;

typedef struct circular_buffer
{
    int stop;
    edge_t solutions[MAX_BUFFER_INDEX][9];
} circular_buffer_t;