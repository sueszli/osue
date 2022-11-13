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
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <ctype.h>
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

/* open the semaphores */
sema_t *createSema(void);

/* open the sharedmemory */
updateC_t *createCircular(void);

/* standard usage function */
void usage(char *c);

/* free all the resources */
void free_resources(updateC_t *circular,sema_t *semaphores);