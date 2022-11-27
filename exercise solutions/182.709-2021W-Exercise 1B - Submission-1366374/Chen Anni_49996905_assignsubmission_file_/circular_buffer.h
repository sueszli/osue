/**
 * @file circular_buffer.h
 * @author Anni Chen
 * @date 06.11.2021
 * @brief module which handles setup of the circular buffer as well as writing and reading from it
 * @details This module provides functions to open/close a circular buffer and reading/writing from/to it. Semaphores are also initialised here.
 * The functions are used in the programs generator and supervisor.
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "3color.h"
#include <semaphore.h>

/**
 * @brief The limit of data the circular buffer can store
 */
#define MAX_DATA 50

#define SHM_NAME "/11902050_shm_name"
#define SEM_FREE "/11902050_sem_free"
#define SEM_USED "/11902050_sem_used"
#define SEM_WRITE "/11902050_sem_write"

/**
 * @brief circular buffer
 * @details it wraps an array which contains the solutions, writePos/readPos indicating the positions to write/read to/from,
 * the state of the circular buffer which shows whether the circular buffer is closed or not
 */
struct Circbuf
{
    struct Solution solutions[MAX_DATA];
    int writePos;
    int readPos;
    int state;
};

struct Circbuf *circbuf;

/**
 * @brief semaphore for writing operations (free space)
 */
sem_t *sem_free;

/**
 * @brief semaphore for reading operations (used space)
 */
sem_t *sem_used;

/**
 * @brief semaphore for writing operations (makes writing atomic)
 */
sem_t *sem_write;

/**
 * opens the circular buffer.
 * @brief This function opens the circular buffer as a shared memory
 * @details The role specified determines how to set up the buffer. If a server (supervisor) calls the function, the shared memory will
 * be created and the struct members of the circular buffer will be initialised: state will be set to 1 to indicate, that the
 * buffer exists, writePos and readPos both set to 0 to indicate that they point to the start of the buffer. If a client (generator) calls this
 * function the existing memory will be opened.
 * @param role either 's' or 'c' allowed where 's' means that the server and 'c' means that the client is calling the process
 * @return the file discriptor of the shared memory, in case of an error -1.
 */
int open_circbuf(char role);

/**
 * closes the circular buffer.
 * @brief This function closes the circular buffer
 * @details If the server (supervisor) calls the function, the state of the circular buffer will be set to 0 which then stops all generator
 * processes and the connection to the buffer will be closed. If it is called by the client (generator), the connection to the buffer will just be closed.
 * Both roles also close all semaphores. In case that the server calls the function the semaphores will additionally be unlinked.
 * @param role either 's' or 'c' allowed where 's' means that the server and 'c' means that the client is calling the process
 * @return 0 if all closing operations are successfull, otherwise -1.
 */
int close_circbuf(char role);

/**
 * initialises the semaphores
 * @brief This function initalises the semaphores
 * @details If the server (supervisor) calls the function, new semaphores will be created and sem_free, sem_used, sem_write will be set.
 * The client (generator) only opens the existing semaphores
 * @param role either 's' or 'c' allowed where 's' means that the server and 'c' means that the client is calling the process
 * @return 0 if all opening operations are successfull, otherwise -1.
 */
int init_sem(char role);

/**
 * reads a solution from the circular buffer
 * @brief This function reads a solution from the buffer
 * @details sem_used blocks the process if there are no entries to read from the buffer. The function waits therefor for
 * a solution to read. After reading, it increases the readPos pointer by one to point to the next position to read from. In the end
 * it performs a sem_post to sem_free to indicate that one position is now free
 * @param role either 's' or 'c' allowed where 's' means that the server and 'c' means that the client is calling the process
 * @return a solution which status is 0 upon success and -1 upon failure
 */
struct Solution read_solution(void);

/**
 * writes a solution to the circular buffer
 * @brief This function writes a solution to the buffer
 * @details sem_write blocks other processes if there is already a process writing to the buffer. This ensures mutal exclusion so
 * that only one process can write to the buffer at the same time. sem_free blocks the process if there are no spaces available to
 * write to. After writing, the writePos will be increased by one which then indicates the next position of the buffer to write to.
 * In the end, sem_post will be performed to sem_used and sem_write to indicate that one more space is now occupied and that the writing
 * process has ended.
 * @param role either 's' or 'c' allowed where 's' means that the server and 'c' means that the client is calling the process
 * @return 0 upon success and -1 upon failure
 */
int write_solution(struct Solution solution);

/**
 * returns the state of the circular buffer
 * @brief This function returns the state of the circular buffer
 * @details The states can be either 1 or 0, indicating that the buffer is available or not
 * @return 1 if the buffer is alive, 0 if not
 */
int get_state(void);