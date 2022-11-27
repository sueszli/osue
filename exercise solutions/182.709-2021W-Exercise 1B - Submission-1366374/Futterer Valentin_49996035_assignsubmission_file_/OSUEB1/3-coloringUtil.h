/**
 * @file 3-coloringUtil.h
 * @author Valentin Futterer 11904654
 * @date 01.11.2021
 * @brief Provides methods for mygrep.c to use.
 * @details The error and usage function are provided by this file. It also provides the core functionality for mygrep.c.
 * The mygrep method reads lines from the input and uses search_str to search the line for a keyword. If one is found,
 * it is printed to the output file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#define SHM_SIZE 512
#define MAX_EDGES_IN_SOLUTION 8
#define SHM_NAME "/11904654_circ_buf"
#define SEM_READ "/11904654_sem_read"
#define SEM_WRITE "/11904654_sem_write"
#define SEM_MUT_EXCL "/11904654_mut_excl"

/**
 * @typedef Edge
 * @struct Edge
 * @brief Represents an edge with start and end node
*/
typedef struct Edge {
    unsigned int start;
    unsigned int end;
} Edge;

/**
 * @typedef Removed_edges
 * @struct Removed_edges
 * @brief Represents a solution
 * @details Solution is represented by an array of Edges with size MAX_EDGES_IN_SOLUTION
*/
typedef struct Removed_edges {
    Edge removed_edges[MAX_EDGES_IN_SOLUTION];
} Removed_edges;

/**
 * @typedef Circ_buf
 * @struct Circ_buf
 * @brief Represents the Circular buffer used by supervisor and generator
 * @details quit_flag signals the generators to exit when set to 1. write_pos signals which
 * position to write to. solution is an array of Removed_edges and solution_size assigns the size of the solution
 * to each entry.
*/
typedef struct Circ_buf {
    sig_atomic_t quit_flag;
    unsigned int write_pos;
    unsigned short solution_size[(SHM_SIZE - sizeof(sig_atomic_t) - sizeof(unsigned int) - sizeof(unsigned short)*MAX_EDGES_IN_SOLUTION) / sizeof(Removed_edges)];
    Removed_edges solution[(SHM_SIZE - sizeof(sig_atomic_t) - sizeof(unsigned int) - sizeof(unsigned short)*MAX_EDGES_IN_SOLUTION) / sizeof(Removed_edges)];
} Circ_buf;

//explicit declaration
/**
 * @typedef Colors
 * @enum Colors
 * @brief Represents the colors, red, green and blue used to color the graph
*/
typedef enum{
    RED = 0,
    GREEN = 1,
    BLUE = 2
} Colors;

/**
 * @brief Exits with error.
 * @details Prints a message to stderr and prints the error code from errno, exits with error.
 * @param prog_name Programme name to print in error message.
 * @param msg The error message to print.
 * @return Returns nothing.
*/
void handle_error(const char *prog_name, char *msg);


/**
 * @brief Opens the circular buffer used by supervisor and generator.
 * @details Calls shm_open and truncates the memory area. Then it maps the memory area, closes the file
 * descriptor and returns the circular buffer.
 * @param prog_name Programme name to print in possible error messages.
 * @return Returns the circular buffer as shared memory.
*/
Circ_buf* open_circular_buffer(const char *prog_name);