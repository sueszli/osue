#include <stdio.h> // needed for fopen, fclose, fprintf
#include <semaphore.h> // needed for semaphores (sem_t)
#include <errno.h> // needed for error number constants
#include <stdbool.h> // needed for boolean values
#include <stdlib.h> // for exit codes
#include <sys/mman.h>	// for shm_unlink
#include <sys/stat.h> 	// for mode contants (shm)
#include <fcntl.h>	// for O_* constants
#include <errno.h>	// für errno
#include <unistd.h>	// for closing shared memory

#ifndef SHM
#define SHM "/01425044_SHM_test6"
#endif

#ifndef FREE_SPACE_SEM
#define FREE_SPACE_SEM "/01425044_FREE_SPACE_SEM_test6"
#endif

#ifndef USED_SPACE_SEM
#define USED_SPACE_SEM "/01425044_USED_SPACE_SEM_test6"
#endif

#ifndef WRITING_SEM
#define WRITING_SEM "/01425044_WRITING_SEM_test6"
#endif

#ifndef TERMINATION_SEM
#define TERMINATION_SEM "/01425044_TERMINATION_SEM_test6"
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 5
#endif

#ifndef LIMIT_EDGES
#define LIMIT_EDGES 8
#endif

#ifndef MODE_WRITE
#define MODE_WRITE 1
#endif

#ifndef MODE_SHUTDOWN 
#define MODE_SHUTDOWN 0
#endif

// von edge durch node ersetzt worden sind, ausser beim result!
/**
 * @brief struct to save a directed edge of a graph (node1 --> node2)
 * */
typedef struct edge { 
	int node1;
	int node2;
} edge;

/**
 * @brief struct to save an result/solution
 * */
typedef struct result {
	edge removed_edges[LIMIT_EDGES];	// array with all the removed edges in it
} result;

/**
 * @brief core of the circular buffer; index of head/write is saved in head; index of tail/read is saved in tail; MODE_WRITE indicates that the generators are allowed to write, MODE_SHUTDOWN indicates that all generators shall shutdown because the supervisor is shutting down also;
 * */
typedef struct buffer {
	int mode;
	result results[BUFFER_SIZE];
	int head; /* will be increment whenever a value is written into the buffer */
	int tail; /* will be decremented whener a value is read from the buffer */
	int seed_offset; /* will be used to set an offset to the seed for the random number generator */
} buffer;

/**
 * @brief call to get a seed offset that was not used yet
 * @details returns an seed offset and changes the currently stored seed offset to prevent using the same offset twice
 * @return seed offset (int) or -1 if an error occurs
 * */
int get_seed_offset(void);

/**
 * @brief writes array into buffer
 * @details writes array into buffer
 * @param result this array will be written into the buffer
 * @return true if successful, false otherwise (in case of an error)
 * */
bool write_buffer(result* result);

/**
 * @brief prints the result
 * @details prints the result (specified form)
 * @param result result that needs to be printed
 * @return void
 * */
void print_result(result* result);


/**
 * @brief reads the next array
 * @details reads the next array and writes it into the given array
 * @param result array where the removed edges from the current tail of the buffer will be written into
 * @return true if successful, false otherwise
 * */
bool read_buffer(result* result);

/**
 * @brief counts edges in given array
 * @details counts edges in giben array that are not NULL
 * @param result array with edges
 * @return the count of edges that are not NULL
 * */
int count_edges(result* result);

/**
 * @brief initialises the buffer
 * @details tries to initialize the buffer
 * @return void
 * */
void init(bool for_generator);

/**
 * @brief loads buffer for generator
 * @detail does almost the same as init(), but uses different flags to create semaphores
 * @return void
 * */
void load_buffer(void);

/**
 * @brief remove buffer for generator
 * @details does almost the same as terminate(), but does not unlink anything
 * @return void
 * */
void remove_buffer(void);

/**
 * @brief close everything that needs to be closed and tries to tell generators to terminate as well
 * @return void
 * */
void terminate(void);

/**
 * @brief prints an error
 * @details prints an error to stderr
 * @param name program name
 * @param message error message
 * @return void
 * */
void print_error(char* name, char* message);

/**
 * @brief prints the result
 * @details prints the result in a specified way: "Solution with x edges: edge1 edges2 ..."
 * @param result result that needs to be printed
 * return void
 * */
void print_result(result* result);

/**
 * @brief terminates and prints error
 * @details prints error to stderr and terminates with EXIT_FAILURE
 * @param name program name
 * @param message error message
 * */
void terminate_with_error(char* name, char* message);

/**
 * @brief removes buffer used by generator
 * @details prints error to stderr and terminates with EXIT_FAILURE
 * @param name program name
 * @param message error message
 * */
void remove_buffer_with_error(char* name, char*message);

extern sem_t* free_space_sem;
extern sem_t* used_space_sem;
extern sem_t* writing_sem;

/* indicates that supervisor terminated and generators have to terminate as well */
extern sem_t* termination_sem;

extern buffer* buf;
extern int shm_fd;
extern char* name;
