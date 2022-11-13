/**
* @file circular-buffer.h
* @author Laurenz Gaisch <e11808218@student.tuwien.ac.at>
* @date 14.11.2021
*
* @brief Header for the circular-buffer.
* @details Defines all includes and static variables used. Defines the struct that contains the solution.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>

#define MEM_NAME "/11808218_MEM"
#define SEM_FREE "/11808218_FREE"
#define SEM_USED "/11808218_USED"
#define SEM_EXCL "/11808218_EXCL"
#define BUF_SIZE 42
#define MAX_EDGES 8
#define CIRCULAR_BUFFER_NAME "circular-buffer"


//Saves the solution by splitting the edges into nodes. Also saves the count and the program state.
typedef struct shmbuf {
    int firstNodeOfEdge[MAX_EDGES];
    int secondNodeOfEdge[MAX_EDGES];
    int count;
    int state; //0 for starting, 1 for running, 2 for ending
} shmbuf;

/**
 * Used by the generator. The solution is saved in the sharedmemory, using the FREE and EXCL semaphores.
 * @param firstNodes
 * @param secondNodes
 * @param listCount
 */
void writeBuffer(int *firstNodes,int *secondNodes,int listCount);

/**
 * Creates the shared-memory on supervisor start.
 */
void createMemory();
/**
 * Opens the shared-memory on generator start.
 */
void openMemory();
/**
 * If the supervisor program was closed by the user, terminate all generators aswell.
 * @param signal
 */
void handle_signal(int signal);

/**
 * Used by the supervisor. Reads the buffer from the shared-memory.
 * @return
 */
shmbuf* readBuffer();

/**
 * Checks whether the program state was set to 2 -> terminated
 * @return state
 */
int stillRunning();

/**
 * Clean the generator
 */
void disposeRes();

/**
 * Clean the generator
 */
void disposeSuper();

//shared memory pointer
shmbuf *shmp;

//semaphores used to access the shared memory
sem_t *semFree;
sem_t *semUsed;
sem_t *semExcl;