/**
 * @file 3coloring.h
 * @author Nursultan Imanov <01528474@student.tuwien.ac.at
 * @date 01.11.2021
 * @brief 
 * @details 
 */

#define SHM_NAME "/01528474_myshm"
#define MAX_DATA (8)
#define BUFFER_LENGTH 100
#define MAX_EDGES 8

#define SEM_1 "/01528474_sem_1"
#define SEM_2 "/01528474_sem_2"
#define SEM_3 "/01528474_sem_3"

/**
 * @brief structure to store the edges of the graph
 * 
 */
typedef struct edge_s
{
  unsigned int from;
  unsigned int to;
} edge_t;

/**
 * @brief structure to store solutions of the graph
 * 
 */
typedef struct solution_s
{
  edge_t edge[MAX_EDGES];
  unsigned int edges_count;
} solution_t;

/**
 * @brief structure to store the data in shared memory
 * 
 */
struct buffer
{
  solution_t solutions[BUFFER_LENGTH];
  unsigned int rd_pos;
  unsigned int wr_pos;
  unsigned int min_edges;
  int quit;
};
