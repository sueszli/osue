/**
 * @file circularbuffer.h
 * @author Lena Jankoschek - 1209852
 * @brief header file for circularbuffer
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */


//to use sem_t
#include <semaphore.h>

//guards
#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H



/**
 * @brief definition of max data -> the size of the data of the shared memory/the circular buffer 
 * and max size solution -> the max number of removed edges in a solution
 * 
 */
#define MAX_DATA (12)
#define MAX_SIZE_SOLUTION (8)


/**
 * @brief a struct which represents the circularbuffer
 * @details the struct includes 3 pointers to semaphores and a pointer to a struct myshm, which represents a shared memory.
 * The free space semaphore represents how much free space there is in the circular buffer.
 * The used space semaphore represents how much space in the circular buffer is used.
 * The write semaphore represents, that only one generator can write at the time.
 */
struct circularbuffer {
    sem_t *sem_free_space;
    sem_t *sem_used_space;
    sem_t *sem_write;
    struct myshm *shm;
};


/**
 * @brief a struct which represents a solution found by a generator
 * @details the struct includes the number of edges of a solution (size) and the edges themself, represented by a two dimensional array.
 * The array can include MAX_SIZE_SOLUTION (8) removed edges max. Each edge holds 2 nodes.
 */
struct solution {
    int size;
    int edges[MAX_SIZE_SOLUTION][2];
};


/**
 * @brief a struct which represents a shared memory
 * @details the struct includes an array of MAX_DATA solutions, an integer which represents the current writing position (of the buffer),
 * an integer which represents the current reading position (of the buffer) and an integer which represents the current state of the buffer/the shared memory.
 * If the state is 0, than everything is okay. If the state != 0 than the circularbuffer should be closed and the programs should be exited.
 */
struct myshm {
    struct solution data[MAX_DATA];
    int write_pos;
    int read_pos;
    int state;
};


/**
 * @brief this function creates or opens (coo) a circularbuffer and returns the buffer.
 * @details depending on who called this function, a circular buffer is created or opened. 
 * If the supervisor calles this function (caller = 's') the buffer and with it the 3 semaphore and the shared memory. 
 * If a generator calls this function (caller = 'g') it returns the buffer, if it was already created by the 
 * supervisor, otherwise an error is thrown. It also opens the semaphores and the shared memory.
 * @param caller - a char which represents from which program the function was called. 's' = function was called from superviser, 
 * 'g' = function was called from generator.
 * @return struct circularbuffer* - the circularbuffer which was created (caller = s) or opened (caller = g) by the function
 */
struct circularbuffer *cb_coo(char caller);


/**
 * @brief this function closes the circularbuffer and all its resources.
 * @details the function frees the circularbuffer, the semaphores und unmaps the shared memory. 
 * If the supervisor is the caller of the functon (caller = 's'), then it also unlinks the semaphores and the shared memory.
 * @param circubuf - pointer to the circularbuffer that should be closed
 * @param caller - a char which represents from which program the function was called. 's' = function was called from superviser, 
 * 'g' = function was called from generator.
 * @return int - the return code which which the function exits. 0 represents that everything is okay. If the return code is not 0, than an error occured
 */
int cb_close(struct circularbuffer *circubuf, char caller);


/**
 * @brief this function writes a solution into the circularbuffer
 * @details the function writes a solution into the circularbuffer. It waits until it is allowed to write (the write semaphore "is free" and there
 * is free space in the circularbuffer). Until then it blocks. If it is its turn to write, the given solution is written into the cirularbuffer
 * on the current write position.
 * @param circbuf - pointer to the circularbuffer which should be written to
 * @param sol - the solution which is written into the buffer
 * @return int - the return code which which the function exits. 0 represents that everything is okay. If the return code is not 0, than an error occured
 */
int cb_write(struct circularbuffer *circbuf, struct solution sol);


/**
 * @brief this function reads a solution from the circularbuffer
 * @details the function reads a solution from the circularbuffer from the current read position. It waits until there is something to read
 * from the buffer (used space semaphore is not 0). Until then it blocks. 
 * @param circbuf - pointer to the circularbuffer that should be read from
 * @return struct solution - the solution which was read from the buffer. if sol.size = -1 -> no solution in buffer
 */
struct solution cb_read(struct circularbuffer *circbuf);


#endif