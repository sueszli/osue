/**
 * @file 3coloring.h
 * @author Andreas Himmler 11901924
 * @date 12.11.2021
 *
 * @brief Methods to check if certain outputs returned an error and prints an
 *        appropriate message for those.
 */
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define FREE_SPACE "/11901924_free"
/**< Name of the free space semaphore */
#define USED_SPACE "/11901924_used"
/**< Name of the used space semaphore */
#define MUTEX_NAME "/11901924_mutex"
/**< Name of the buffer mutex semaphore */
#define BUFFER_NAME "/11901924_cb"
/**< Name of the circular buffer shared memory */
#define BUFFER_SIZE 50
/**< Size of the circular buffer (Number of solutions it can hold) */
#define MAX_EDGES 8
/**< Maximum of edges a solution can have */

/**
 * @brief Structure to define an edge
 */
typedef struct edge {
    int v1;
    int v2;
} edge;

/**
 * @brief Structure to define an solution (Set of edges an the count of those)
 */
typedef struct solution {
    edge edges[MAX_EDGES];
    int count;
} solution;

/**
 * @brief Structure to represent the circular buffer. Contains the buffer of
 *        solutions. The current read and write positions, an flag, if the
 *        generators should terminate and a number for the count of edges of
 *        the best solution.
 */
typedef struct circular_buffer {
    solution solutions[BUFFER_SIZE];
    int read_pos;
    int write_pos;
    int alive;
    int best_sol;
} circular_buffer;

/**
 * @brief Structure containing all necessary shared memorys and semaphores.
 */
typedef struct process {
    int bufferfd;
    circular_buffer *buffer;
    sem_t *free_space;
    sem_t *used_space;
    sem_t *buff_mutex;
} process;

int check_buffer(int, char*, char*);
int check_close(int, char*, char*);
int check_sem_open(sem_t*, char*, char*);
int check_sem_close(int, char*, char*);
int check_sem_unlink(int, char*, char*);
void print_map_error(char*, char*);
int check_sem_post(int, char*, char*);
int check_sigaction(int, char*);

