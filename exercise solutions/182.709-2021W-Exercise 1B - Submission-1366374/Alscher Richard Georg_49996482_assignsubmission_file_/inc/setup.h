/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * @module                                                                              *
 *                                                                                      *
 * @author  Richard Alscher - 11775285                                                  *
 *                                                                                      *
 * @brief   This is the setup header file for the feedback_arc_set algorithm            *
 * @details This file includes all the required libraries, sets up some shared          *
 *          datastructures and sets some shared variables.                              *
 *                                                                                      *
 * @date    14.11.2021                                                                  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "functions.h"

#define MAX_EDGES (10)
#define MAX_VERTICES (2 * MAX_EDGES)
#define SHM_NAME "/11775285_share"
#define MAX_DATA (5)
#define SEM_MUTEX "/11775285_mutex"
#define SEM_FREE "/11775285_freeSem"
#define SEM_USED "/11775285_usedSem"

typedef struct Edge {
    unsigned int start;
    unsigned int end;
} Edge;

typedef struct Graph {
    Edge edges[ MAX_EDGES ];
    int edgeCount;
    int vertices[ MAX_VERTICES ];
} Graph;

struct sharedMemory {
    unsigned int state;
    unsigned int readPos;
    unsigned int writePos;
    Graph data[ MAX_DATA ];
};

volatile sig_atomic_t terminate;
static int shmfd;
struct sharedMemory *sharedMemory;
sem_t *mutex;
sem_t *freeSem;
sem_t *usedSem;

char *programName;
