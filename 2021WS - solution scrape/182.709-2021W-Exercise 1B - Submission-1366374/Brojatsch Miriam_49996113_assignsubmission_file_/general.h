/**
* @file general.h
* @author Miriam Brojatsch 11913096
* @date 14.11.2021
*
* @brief This headerfile contains library links, constants and structs for use in generator.c and supervisor.c.
**/


#ifndef GENERAL_H
#define GENERAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/mman.h>
#include <fcntl.h> //for open
#include <unistd.h> //for close
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h> //for size_t


//shared memory file descriptor
static int shm_fd = -1;

//semaphores
static sem_t * sem_used = NULL;
static sem_t * sem_free = NULL;
static sem_t * sem_mutex = NULL;

//constants
#define MATRICULATION_NO "/11913096" //name for shared memory
#define BUFFER_SIZE 25
#define SEM_FREE_NAME "/11913096_free"
#define SEM_USED_NAME "/11913096_used"
#define SEM_MUTEX_NAME "/11913096_mutex"


//edge type
typedef struct {
    int u;
    int v;
} edge;

//solution type
typedef struct {
    int size;
    edge edges[7]; //max accepted number of edges is 8
} solution_set;

//circular buffer type
typedef struct {
    //the actual buffer
    solution_set data[BUFFER_SIZE];

    //next position to be read, unsigned so it really cannot get <0, even if i forget to check
    unsigned int read_at;

    //next position to be written to, unsigned so it really cannot get <0, even if i forget to check
    unsigned int write_at;

    //flag if supervisor wants generators to stop, unsigned because why not
    unsigned int please_exit;
} circular_buffer;


//global pointer to buffer
static circular_buffer *buffer = NULL;

#endif