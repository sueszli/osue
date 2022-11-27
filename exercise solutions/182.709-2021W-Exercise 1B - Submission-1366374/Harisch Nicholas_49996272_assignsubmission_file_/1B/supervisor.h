/**
 * @file supervisor.h
 * @author Nicholas Harisch 11705361<harischnicholas@gmail.com>
 * @date 12.11.2021
 *
 * @brief Util for supervisor.c and generator.c
 **/
#include <stdio.h>
#include <signal.h>

#define MAX_SIZE 50 //*< max shared memory buffer size */
#define SOLUTION_MAX 8 //*< max solution size that can get proposed */

//Semaphores
#define SEM_FREE "/11705361_sem_free"
#define SEM_USED "/11705361_sem_used"
#define SEM_BLOCK "/11705361_sem_block"
#define SHM_NAME "/11705361_supervshm"

//struct for an edge
struct Edge
{
    int a, b;
};

//struct for a single solution (set of edges)
struct Solution
{
    ssize_t size;
    struct Edge edges[8];
};

//Struct for the shared memory ring buffer
struct Ring_Buffer
{
    int status;
    int rd_pos;
    int wr_pos;
    struct Solution solutions[MAX_SIZE];
};
