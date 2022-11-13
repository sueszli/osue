/**
 * @file generator.h
 * @author Philipp Holzer <e12028208@student.tuwien.ac.at>
 * @date 2021-11-14
 * @brief includes headers and structs used in generator.c and supervisor.c
 **/

#include <sys/wait.h>
#include <sys/mman.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <semaphore.h>

#define SHMNAME "/shm12028208"
#define BSIZE 25
#define SFREE "/free12028208"
#define SUSED "/used12028208"
#define SMUTEX "/mutex12028208"

/**
 * @brief represents a directed edge by vertices (from -> to)
 */
typedef struct {
    int f; // from 
    int t; // to
} edge;

/**
 * @brief edgec = edge container, used for storing possible solutions (with a capacity of 8 (according to exercise)).
 */
typedef struct {
    edge container[8]; // edge container
    int counter; // number of edges
} edgec;

/**
 * @brief shared memory for reading and writing possible solutions (used by supervisor and generator)
 * @details contains edgec array (size = BSIZE)
 */
typedef struct {
    edgec data[BSIZE]; // edgec array containing possible solutions
    int read; // pointer to the next position, that hasn't been read yet
    int write; // the next free position (that can be written, if the array isn't full)
    int shutdown; // flag to initiate shutdown
    int generatorcnt; // number of generators running (important for shutdown) 
} circbuffer;