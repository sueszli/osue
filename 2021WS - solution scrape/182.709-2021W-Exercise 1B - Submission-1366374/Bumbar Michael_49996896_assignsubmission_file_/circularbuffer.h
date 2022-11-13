/**
 * @file circularbuffer.h
 * @author Michael Bumbar <e11775807@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief Header file of the circularbuffer module.
 * 
 * This file contains the declarations of all non-static functions in circularbuffer.c. These functions are related to opening a shared memory, POXIX semaphores, writing and reading
 * the shared memory and closing all shared resources. Additionally it was used to declare a common usage and error handling function. 
 * It also contains the defintion of all data structures used in supervisor.c, generator.c and circularbuffer.c. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

#define MAX_SIZE (32) /**< The maximal size of the circular buffer. */
#define NAME "/11775807_buffer" /**< The name of the circular buffer shared memory file. */
#define SEM1 "/11775807_sem_1" /**< The name of the POSIX semaphores that stores the number of free spaces in the circular buffer. */
#define SEM2 "/11775807_sem_2" /**< The name of the POSIX semaphores that stores the number of occupied spaces in the circular buffer. */
#define SEM3 "/11775807_sem_3"  /**< The name of the POSIX semaphores that is used for the mutual exclusion of the generator processes accessing the circular buffer. */
#define GEN "./generator" /**< The name of the generator.c program. Used for error Handling. */
#define SUP "./supervisor" /**< The name of the supervisor.c program. Used for error Handling. */


/** 
 *  @enum activity
 *  @brief It is set to the current state of the supervisor. An INACTIVE supervisor is about to terminate.
 */
typedef enum {
    ACTIVE = 0, /**< enum value ACTIVE. */ 
    INACTIVE = 1 /**< enum value INACTIVE. */ 
} activity;


/** @struct edge
 *  @brief This structure is used to store an edge.
 *  @var edge::start
 *  Member 'start' contains the starting vertex of the edge.
 *  @var edge::end 
 *  Member 'start' contains the ending vertex of the edge.
 */
typedef struct {
    int32_t start;
    int32_t end;
} edge;


/** @struct arc_set
 *  @brief This structure is used to store a feedback arc set.
 *  @var arc_set::result
 *  Member 'result' contains the edges of a feedback arc set. Its length is 8.
 *  @var arc_set::end 
 *  Member 'length' contains the number of edges in a solution.
 */
typedef struct {
    edge result[8];
    int32_t length;
} arc_set;


/** @struct cicbuffer
 *  @brief This structure constitutes a circular buffer.
 *  @var cicbuffer::buffer
 *  Member 'buffer' contains the space used to write and read solutions to a graph.
 *  @var cicbuffer::write_pos  
 *  Member 'write_pos' is the current writing position in the circular buffer.
 *  @var cicbuffer::read_pos  
 *  Member 'read_pos' is the current reading position in the circular buffer.
 *  @var cicbuffer::running  
 *  Member 'running' is the current activity status of the supervisor using this structure.
 */
typedef struct {
    arc_set buffer[MAX_SIZE];
    int32_t write_pos;
    int32_t read_pos;
    activity running;
} circbuffer;


/**
 * Sets up shared memory and opens it.
 * @brief Sets up shared memory space and opens it in the supervisor process. Calls open_semaphores to create POSIX semaphores.
 * @details This function starts by creating a shared memory and mapping it into its memory space. The file descriptor used for the creation of the shared memory is closed and open_semaphores is
 * called to create POSIX semaphores. The shared memory is set to read and write and the function creates the object if no file with the same name already exists.
 * Additionally running in the circular buffer is set to ACTIVE, write_pos and read_pos are set to 0.
 * The function returns with EXIT_SUCCESS
 * @return Returns EXIT_SUCCESS.
 */
int32_t setup_buffer(void);


/**
 * Opens a previously created shared memory.
 * @brief Opens already created shared memory space in the generator process. Calls open_semaphores to open previously created POSIX semaphores.
 * @details This function starts by opening a shared memory and mapping it into its memory space. The file descriptor used for the opening of the shared memory is closed and open_semaphores is
 * called to create POSIX semaphores.
 * The function returns with EXIT_SUCCESS
 * @return Returns EXIT_SUCCESS.
 */
int32_t load_buffer(void);

/**
 * Unnmaps and unlinks shared memory.
 * @brief Unnmaps and undlinks shared memory space and calls free_semaphores to close and unlink POSIX semaphores.
 * @return Returns EXIT_SUCCESS.
 */
int32_t free_original_buffer(void);


/**
 * Unnmaps shared memory.
 * @brief Unnmaps shared memory and calls free_semaphores to close POSIX semaphores.
 * @return Returns EXIT_SUCCESS.
 */
int32_t free_loaded_buffer(void);


/**
 * Changes running in the circular buffer.
 * @brief Changes the activity of running in circular buffer according to the actions of the supervisor.
 * @details This function checks if running is ACTIVE and if so, sets it to INACTIVE.
 */
void activity_change(void);


/**
 * Reads from the circular buffer.
 * @brief Reads a solution feedback arc set from the circular buffer.
 * @details The supervisor calling this function will check wether there are occupied spaces in the circular buffer. If so the solution will be read from 
 * the buffer and the semaphores for writing the buffer is incremented as this function terminates. If a signal interrupts this function the generator returns with -1.
 * constants: SUP
 * @param s Solution feedback arc set.
 * @return Returns EXIT_SUCCESS.
 */
int32_t read_buffer(arc_set* s);


/**
 * Writes to the circular buffer.
 * @brief Writes a solution feedback arc set to the circular buffer.
 * @details If no generator is writing to the circular buffer the generator calling this function will check wether there are free spaces in the circular buffer. If so the solution will be written to 
 * the buffer and the semaphores for reading the buffer and mutual exclusion are incremented as this function terminates.
 * constants: SUP
 * @param s Solution feedback arc set.
 * @return Returns EXIT_SUCCESS.
 */
int32_t write_buffer(arc_set s);


/**
 * Compares two edges.
 * @brief Checks wether the start and end vertex of two edges are equal.
 * @details This function returns 0 if the start and end vertex of two edges are equal. Otherwise it returns 1.
 * constants: PROG, SUCCESS, NO_SUCCESS
 * @param a First edge of the comparison.
 * @param b Second edge of the comparison.
 * @return Returns 1.
 */
int32_t cmp_edge(edge* a, edge* b);


/**
 * Prints a feedback arc set.
 * @brief Iterates over the edges in a feedback arcset and prints them to stdout.
 * @details This function starts by iterating over the edges in a arc_set and prints a formatted string to stdout using the edge pattern.
 * @param result A feedback arc set.
 */
void show_solution(arc_set* result);


/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details constants: PROG
 */
void usage(char* use, char* prog);


/**
 * Error handling inside the program.
 * @brief This function prints an error message to stderr and exits with EXIT_FAILURE.
 * @details This function prints a formatted string to stderr. The string consists of the name of the program in progName followed  *y the name of the function which called errorHandling and the cause of the
 * error at the end. The cause is usually the string format of errno. Afterwards this function exits with exit(EXIT_FAILURE).
 * constants: PROG
 * @param progName The name of the program in which the function is executed.
 * @param errorMessage The errorMessage the program is printing.
 * @param cause The cause for the error of the program.
 * @return Returns EXIT_FAILURE.
 */
void errorHandling(char* progName,char* errorMessage,char*cause);

