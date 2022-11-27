/**
 * @file fb_arc_set.h
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 14.11.2022
 *
 * @brief This module provides macros, options, names, types, structs as well as initialization-, closing- and printing- functions
 *        for supervisor and generator
 * 
 *      
 **/


#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants (permission flags) */
#include <fcntl.h>    /* For O-flags */
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <signal.h>


#define SHM_NAME "/11904658_circ_buffer"

#define BUFFER_SIZE 4
#define MAX_NO_EDGES_ALLOWED_IN_SOLUTION 8
#define MAX_NO_GRAPH_EDGES_ALLOWED 100

#define USED_SEM_NAME  "/11904658_sem_used"
#define FREE_SEM_NAME  "/11904658_sem_free"
#define MUTEX_SEM_NAME "/11904658_sem_mutex"

#define USED_SEM_INIT_VALUE  0
#define FREE_SEM_INIT_VALUE  BUFFER_SIZE
#define MUTEX_SEM_INIT_VALUE 1


/** An edge contains the two nodes it connects (direction is implicit; from node_out -> node_in) */
struct edge{
    int node_out;
    int node_in; 
};


/** An edge_set contains an array of various edges (maximum defined in MAX_NO_EDGES_ALLOWED_IN_SOLUTION) and its length. */
struct edge_set{
    struct edge edge_array[MAX_NO_EDGES_ALLOWED_IN_SOLUTION]; 
    int length;
};

/** A graph contains an array of edges (maximum defined in MAX_NO_GRAPH_EDGES_ALLOWED) and its length */
struct graph{
    struct edge edge_array[MAX_NO_GRAPH_EDGES_ALLOWED]; 
    int length;
};

/** A circular_buffer is an "BUFFER_SIZE"-entry array for edge-sets. */
struct circular_buffer{
    struct edge_set buffer_array[BUFFER_SIZE];
    int write_end;
    int read_end;
    bool quit_flag_generators;
};

typedef struct circular_buffer circular_buffer_t;


/**
 * @brief This function creates and and initializes a shared memory and is therefore called by the supervisor. 
 * @details The function calls the following other functions: shm_open(3), ftruncate(2), mmap(2)
 *          If any of the called functions fails then this function closes the opened resources and terminates the supervisor
 *          with EXIT_FAILURE.
 *
 * @param fd           The pointer to the memory space where the file descriptor of the created shared memory object is stored. 
 * @param circ_buffer  The double pointer to the memory space of the circular buffer.  
 */
void initialize_shared_mem_as_server(int *fd, circular_buffer_t **circ_buffer);

/**
 * @brief This function opens a created shared memory object and is therefore called by the generator. 
 * @details The function calls the following other functions: shm_open(3), mmap(2)
 *          If any of the called functions fails then this function closes the opened resources and terminates the generator
 *          with EXIT_FAILURE.
 *
 * @param fd           The pointer to the memory space where the file descriptor of the shared memory object is stored. 
 * @param circ_buffer  The double pointer to the memory space of the circular buffer.  
 */
void initialize_shared_mem_as_client(int *fd, circular_buffer_t **circ_buffer);

/**
 * @brief This function unmaps, closes and unlinks a beforehand created shared memory object and is therefore called by the supervisor. 
 * @details The function calls the following other functions: munmap(2), close(2), shm_unlink(3)
 *          If any of the called functions fails then this function terminates the supervisor
 *          with EXIT_FAILURE.
 *
 * @param fd           The file descriptor of the shared memory object. 
 * @param circ_buffer  The pointer to the memory space of the circular buffer.  
 */
void close_shared_mem_as_server(int fd, circular_buffer_t *circ_buffer);

/**
 * @brief This function unmaps and closes a beforehand created shared memory object and is therefore called by the generator. 
 * @details The function calls the following other functions: munmap(2), close(2)
 *          If any of the called functions fails then this function terminates the generator
 *          with EXIT_FAILURE.
 *
 * @param fd           The file descriptor of the shared memory object. 
 * @param circ_buffer  The pointer to the memory space of the circular buffer.  
 */
void close_shared_mem_as_client(int fd, circular_buffer_t *circ_buffer);



/**
 * @brief This function creates and initializes a semaphore to be used for synchronisation purposes between supervisor and generator
 *        or between two generators. It is called by the supervisor. 
 * @details The function calls the following other functions: sem_open(3)
 *          If any of the called functions fails then this function terminates the supervisor
 *          with EXIT_FAILURE.
 *
 * @param sem         The double pointer to the semaphore to be created. 
 * @param sem_name    The name of the semaphore to be created, handed over as a C-String.
 * @param init_value  The initial value of the semaphore to be created.
 */
void initialize_sem_as_server(sem_t** sem, const char *sem_name, unsigned int init_value);

/**
 * @brief This function opens an already created semaphore to be used for synchronisation purposes between supervisor and generator
 *        or between two generators. It is called by the generator. 
 * @details The function calls the following other functions: sem_open(3)
 *          If any of the called functions fails then this function terminates the generator
 *          with EXIT_FAILURE.
 *
 * @param sem       The double pointer to the semaphore to be opened. 
 * @param sem_name  The name of the semaphore to be opened, handed over as a C-String.
 */
void initialize_sem_as_client(sem_t** sem, const char *sem_name);

/**
 * @brief This function closes and unlinks a beforehand created semaphore and is therefore called by the supervisor. 
 * @details The function calls the following other functions: sem_close(3), sem_unlink(3)
 *          If any of the called functions fails then this function terminates the supervisor
 *          with EXIT_FAILURE.
 *
 * @param sem       The double pointer to the semaphore to be closed and unlinked. 
 * @param sem_name  The name of the semaphore to be closed and unlinked, handed over as a C-String.
 */
void remove_sem_as_server(sem_t *sem, const char *sem_name);

/**
 * @brief This function closes a beforehand created semaphore and is therefore called by the generator. 
 * @details The function calls the following other functions: sem_close(3)
 *          If any of the called functions fails then this function terminates the generator
 *          with EXIT_FAILURE.
 *
 * @param sem  The double pointer to the semaphore to be closed.
 */
void remove_sem_as_client(sem_t *sem);


/**
 * @brief This function prints the edges contained in the provided edge_set to stdout. 
 *
 * @param set            The pointer to the edge_set to be printed
 * @param string_before  The pointer a string the be printed before the edge_set.
 */
void print_edge_set(struct edge_set *set, char *string_before);