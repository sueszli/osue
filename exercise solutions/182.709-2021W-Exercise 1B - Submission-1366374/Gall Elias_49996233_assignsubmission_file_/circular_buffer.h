#ifndef CIRCULAR_BUFFER_H /* include guard */
#define CIRCULAR_BUFFER_H

/**
 * @file
 * @author Elias GALL - 12019857
 * @brief header file for circular_buffer.c
 * @details Exposes functions in circular_buffer.c for use in other modules.
 * @date 2021-11-02
 */

/** @brief maximum number of edges a solution can contain */
#define MAX_RESULT_EDGES (8)

/** @brief length of the buffer */
#define BUFFER_LENGTH (64)

/** @brief file name of the shared memory */
#define BUFFER_NAME ("/12019857_circular_buffer")

/** @brief file name of the semaphore limiting writing space */
#define WRITE_SPACE_SEMAPHORE_NAME ("/12019857_wr_sp_semaphore")

/** @brief file name of the semaphore limiting writing access */
#define WRITE_ACCESS_SEMAPHORE_NAME ("/12019857_wr_ac_semaphore")

/** @brief file name of the semaphore limiting reading access */
#define READ_ENTRIES_SEMAPHORE_NAME ("/12019857_rd_en_semaphore")

/**
 * @brief contains exactly one edge defined by it's start and end vertex
 */
typedef struct edge {
    int from;
    int to;
} edge_t;

/**
 * @brief defines one entry in the buffer, containing a exactly number_of_edges edges (no more than MAX_RESULT_EDGES)
 */
typedef struct buffer_entry {
    edge_t edges[MAX_RESULT_EDGES];
    int number_of_edges;
} buffer_entry_t;

/**
 * @brief defines the buffer stored in shared memory
 * @details Contains the array of entries, BUFFER_LENGTH elements long, the next index that will be written (write_pos), the next index that will be read (read_pos) and a flag used by the supervisor to notify it's generators that they should terminate (terminate).
 */
typedef struct circular_buffer {
    buffer_entry_t entries[BUFFER_LENGTH];
    int write_pos;
    int read_pos;
    int terminate;
} circular_buffer_t;

/**
 * @brief used by the supervisor to create the shared memory and all semaphores
 * @details Creates a shared memory named BUFFER_NAME and maps it to 'buffer'. Initializes all indices in the buffer to 0. Creates all 3 semaphores. Aborts on the first error.
 * @details Global variables used: buffer, wr_access_sem, wr_space_sem, rd_access_sem, shm_fd
 * @param program_name name of the program for use in error messages
 * @return 0 ... success
 * @return -1 ... failure
 */
int create(const char*);

/**
 * @brief used by the supervisor to close and unlink all resources
 * @details Closes the file descriptor of the shared memory  and removes the shared memory. Unlinks and closes all 3 semaphores. Does not immediately abort after an error, as to close as many resources as possible.
 * @details Global variables used: buffer, shm_fd
 * @param program_name name of the program for use in error messages
 * @return 0 ... success
 * @return -1 ... one or more errors occured
 */
int close_after_create(const char*);

/**
 * @brief used by generators to 'connect' to an existing buffer
 * @details Opens an existing shared memory BUFFER_NAME and maps it to 'buffer'. Opens all 3 existing semaphores. Aborts on the first error.
 * @details Global variables used: buffer, wr_access_sem, wr_space_sem, rd_access_sem
 * @param program_name name of the program for use in error messages
 * @return 0 ... success
 * @return -1 ... failure
 */
int connect(const char*);

/**
 * @brief used by generators to 'disconnect' from shared memory and semaphores
 * @details Unmaps shared memory and closes semaphores. Does not abort on error, as to close as many resources as possible.
 * @details Global variables used: buffer
 * @param program_name name of the program for use in error messages
 * @return 0 ... success
 * @return -1 ... failure
 */
int disconnect(const char*);

/**
 * @brief sets the 'terminate' flag in the buffer
 * @details Sets the 'terminate' flag in the buffer to the value indicated by the parameter.
 * @details Global variables used: buffer
 * @param terminate value of the flag
 * @return void
 */
void set_terminate(const int);

/**
 * @brief gets the 'terminate' flag from the buffer
 * @details Gets the value of the 'terminate' flag in the buffer.
 * @details Global variables used: buffer
 * @return value of 'terminate'
 */
int get_terminate(void);

/**
 * @brief writes 'e' to the buffer
 * @details Writes an entry to the buffer, while using semaphores for synchronisation with other processes and to not exceed the buffer's capacity. 
 * @details Global variables used: buffer, wr_space_sem, wr_access_sem
 * @param e entry to write
 * @param program_name name of the program for use in error messages
 * @return 0 ... success
 * @return -1 ... failure or 'terminate' set
 */
int write_buffer(buffer_entry_t*, const char*);

/**
 * @brief reads entry from the buffer
 * @details Reads the next entry in the buffer and indicates that one more slot is free using 'wr_space_sem'.
 * @details Global variables used: buffer, rd_entries_sem, wr_space_sem
 * @param program_name name of the program for use in error messages
 * @return pointer to a buffer_entry_t instance
 * @return NULL on failure
 */
buffer_entry_t *read_buffer(const char*);

#endif