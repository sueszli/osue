/**
 * @file util.h
 * @author Alexander Gschnitzer (01652750) <e1652750@student.tuwien.ac.at>
 * @date 21.10.2021
 *
 * @brief Utility functions intended to prevent code duplication.
 * @details Collection of type definitions that are used globally as well as definitions of utility functions.
 */

// include guard
#ifndef UTIL_H
#define UTIL_H

/**
 * @brief Names of the shared memory object and semaphores.
 */
#define SHM_NAME "/01652750_shm"
#define SEM_USED "/01652750_used"
#define SEM_FREE "/01652750_free"
#define SEM_ACCESS "/01652750_access"

/**
 * @brief Maximum number of edges contained in a solution.
 */
#define MAX_EDGES 8

/**
 * @brief Maximum size of circular buffer. Should not exceed 4 KiB.
 */
#define MAX_SIZE 16

/**
 * Vertex.
 * @brief Representation of vertex in graph.
 * @details Vertex contains id provided by the input of program and color which is assigned randomly.
 */
typedef struct vertex {
    int id;
    int color;
} vertex_t;

/**
 * Edge.
 * @brief Representation of edge in graph.
 * @details Edge contains ids and colors of adjacent vertices, v1, and v2.
 */
typedef struct edge {
    int v1;
    int v2;
} edge_t;

/**
 * Solution.
 * @brief Representation of a solution to the 3-coloring problem.
 * @details Solution contains n edge sets, i.e. set of edges that need to be removed in order to provide a valid solution -
 * whereas n is defined by MAX_EDGES. Furthermore, it contains the number of removed solutions which is the minimum of the removed edges and MAX_EDGES.
 */
typedef struct solution {
    edge_t edges[MAX_EDGES];
    int removed;
} solution_t;

/**
 * Circular buffer.
 * @brief Representation of shared memory object.
 * @details Circular buffer contains the variable terminate that is used to show the generators that the supervisor wants to termiante the program.
 */
typedef struct circular_buffer {
    solution_t queue[MAX_SIZE];
    int terminate;
    int r_pos;
    int w_pos;
} cb_t;

/**
 * Show usage.
 * @brief Displays usage of program.
 * @details Prints short usage description to stderr and exists the program with EXIT_FAILURE.
 */
void usage(void);

/**
 * Display error.
 * @brief Displays error message.
 * @details Prints error message to stderr and exists program with EXIT_FAILURE. Uses global variable prog_name.
 * @param message custom error message.
 */
void error(const char *message);

/**
 * Prints solution.
 * @brief Prints found solution.
 * @details Displays number of removed edges and each edge in detail, containing ids of both vertices.
 * @param solution valid solution to the 3-coloring problem.
 */
 void print_solution(const solution_t *solution);

/**
 * Initializes shared memory object.
 * @brief Initializes the shared memory object.
 * @details Creates / opens the shared memory object with the FLAGS specified by either supervisor or generator (shm_flags),
 * sets its size (if supervisor calls this function), maps it to a circular buffer object and
 * subsequently closes the file descriptor of the shared memory object - since it will not be used from this point on forward.
 * @param is_supervisor specifies the program that called the function. 0 if generator, or 1 if supervisor.
 */
void init_buffer(int is_supervisor);

/**
 * Cleanup of shared memory object.
 * @brief Clears the shared memory object.
 * @details Unmaps shared memory object, i.e. the circular buffer, and removes it by unlinking it with the name specified in SHM_NAME.
 * @param is_supervisor specifies the program that called the function. 0 if generator, or 1 if supervisor. shm_unlink should only be called once by the supervisor.
 */
void clear_buffer(int is_supervisor);

/**
 * Initializes semaphores.
 * @brief Initializes the three semaphores specified in the exercise sheet.
 * @details Creates / opens all three semaphores with FLAGS specified by either supervisor or generator (sem_flags).
 * If creation failed, program exits with an error.
 * @param is_supervisor specifies the program that called the function. 0 if generator, or 1 if supervisor.
 */
void init_sem(int is_supervisor);

/**
 * Clean up semaphores.
 * @brief Clears the semaphores.
 * @details Closes free-space, used-space and access semaphores. Supervisor unlinks them as well to free up allocated resources.
 * @param is_supervisor specifies the program that called the function. 0 if generator, or 1 if supervisor.
 */
void clear_sem(int is_supervisor);

#endif
