/**
 *@author Gregor Käfer 01326186
 *@brief Write a C-program feedback-arcset.
 *@date ‎Sunday, ‎14 ‎November ‎2021
 */
 

//standard library
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#define CIRCULARBUFFER 8
#define MAXLINE 8 
#define SEM_FREE "/01326186_free"
#define SEM_USED "/01326186_used"
#define SEM_MUTEX "/01326186_mutex"
#define SHM_OP "/01326186_op"

//structure
typedef struct semaphores sema_t;
typedef struct line line_t;
typedef struct solution solution_t;
typedef struct updateCircular updateC_t;

/* handle signal */
void handle_signal(int signal);

/* standard usage function */
void usage(char *c);

/* save input into a line_t type
 * @param input input
 * @param line type structure to be saved into
 * @param filename filename
 */
void saveEdges(char *input,line_t *line,char *filename);

/* open the sharedmemory */
updateC_t *createCircular(void);

/* open the semaphores */
sema_t *createSema(void);

/* free all the resources */
void free_resources(updateC_t *circular,sema_t *semaphores);

/* save all the (i-j) i item to an array
 * @param permutationArray array temp to save the data
 * @param line the saved structure
 * @param index if permutationArray length is changed,index will keep the length in it
 */
void randomPermutation1(int *permutationArray,line_t *line,int *index);

/* save all the (i-j) j item to an array
 * @param permutationArray array temp to save the data
 * @param line the saved structure
 * @param index if permutationArray length is changed,index will keep the length in it
 */
void randomPermutation2(int *permutationArray,line_t *line,int *index);

/* randomize the array
 * @param permutationArray the saved data in array
 * @param index array length
 */
void randomPermutation3(int *permutationArray,int index);

/* swap value */
void swap (int *a, int *b);

/* get the solution with topologicalSort algorithm
 * @param line the structure
 * @param solution save the solution here
 * @param arr the saved randomize data in array
 * @param arrLength array length
 * @param inputEdgesLength how many lines do we get from the input
 */
void getSolution(line_t *line,int *arr,solution_t *solution,int arrLength,int inputEdgesLength);