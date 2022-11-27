/**
 * @file sharedspace
 * @author Jannis Adamek (11809490)
 * @date 2021-11-14
 *
 * @brief Defines common macros and the SharedSpace struct, that are used for
 * interprocess communication. For assignment 1B.
 **/

#ifndef SHARED_SPACE
#define SHARED_SPACE

#include "graph.h"

#include <stdbool.h>

/** Number of slots in the circular buffer */
#define CIRCULAR_BUFFER_SIZE 10

/** Name used for mmap. */
#define SHM_NAME "/11809490_shared_mem"

/** Names of the semaphores. */
#define SEM_FULL "/11809490_sem_full"
#define SEM_FREE "/11809490_sem_free"
#define SEM_MUTEX "/11809490_sem_mutex"

/**
 * Struct that gets maped using mmap.
 */
typedef struct _shared_space {
  EdgeSet buffer[CIRCULAR_BUFFER_SIZE]; /** */
  int rd_pos;
  int wr_pos;
  bool running;
} SharedSpace;

#endif