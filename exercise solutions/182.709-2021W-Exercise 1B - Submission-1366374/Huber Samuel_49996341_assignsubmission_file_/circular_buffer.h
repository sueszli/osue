/**
 * module name: circular_buffer.h
 * @author      Huber Samuel 11905181
 * @brief       header file for circular_buffer.c
 * @details     defines structs, constants, prototypes used
 * @date        08.11.2021
**/

#ifndef UE_1B_CIRCULAR_BUFFER_H
#define UE_1B_CIRCULAR_BUFFER_H

#define MAX_EDGE_COUNT 8   // maximum amount of edges deleted considered for writing into buffer

/**
 * structure representing an edge of a graph
 * @param from: starting point of the edge as node
 * @param to: end point of the edge as node
 */
typedef struct {
    int from;
    int to;
} edge_t;

/**
 * structure containing data for generated solution
 * @param list: list of edges to be deleted for solution
 * @param size: amount of edges to be deleted for solution
 */
typedef struct {
    edge_t list[MAX_EDGE_COUNT];
    int size;
} edgeData_t;


/**
 * @brief initiates buffer
 * @details creates, sets size & maps shared memory,\n
 *          resets buffer, opens semaphores\n
 *          global variables used:\n
 *          'SEM_FREE', 'SEM_USED', 'SEM_MUTEX', 'SHM_NAME',\n
 *          'shmfd', 'terminating', 'buffer', 'free', 'used', 'mutex', 'storageSize'
 */
void initiateBuffer(void);

/**
 * @brief removes buffer
 * @details closes buffer, removes shared memory,\n
 *          removes semaphores\n
 *          global variables used:\n
 *          'SEM_FREE', 'SEM_USED', 'SEM_MUTEX', 'SHM_NAME'
 */
void removeBuffer(void);

/**
 * @brief opens buffer
 * @details creates & maps shared memory,\n
 *          opens semaphores\n
 *          global variables used:\n
 *          'SEM_FREE', 'SEM_USED', 'SEM_MUTEX', 'SHM_NAME',\n
 *          'shmfd', 'buffer', 'free', 'used', 'mutex'
 */
void openBuffer(void);

/**
 * @brief closes buffer
 * @details closes & unmaps shared memory,\n
 *          closes semaphores\n
 *          global variables used:\n
 *          'shmfd', 'buffer', 'free', 'used', 'mutex'
 */
void closeBuffer(void);

/**
 * @brief returns next entry in buffer
 * @details returns entry in circular buffer to which 'reader' points,\n
 *          if buffer is empty, readBuffer waits,\n
 *          global variables used:\n
 *          'used', 'buffer', 'storageSize', 'free'
  * @param readData location of where the read entry will be written to
  */
void readBuffer(edgeData_t *readData);

/**
 * @brief writes new entry into buffer
 * @details writes entry in circular buffer at the position to which 'writer' points,\n
 *          if buffer is full, writeBuffer waits,\n
 *          global variables used:\n
 *          'used', 'buffer', 'storageSize', 'free', 'mutex',
 * @param edges: list of edges to be deleted for solution
 * @param size: amount of edges to be deleted for solution
 */
void writeBuffer(edge_t *edges, int size);


/**
 * @brief sets termination flag in buffer
 * @details if 'isTerminating' is set, it indicates the initiation\n
 *          of program termination for the generator\n
 *          global variables used:\n
 *          's_mutex', 'buffer'
 * @param flag: to what the termination flag should be set
 */
void setTermination(int flag);

/**
 * @brief gets termination flag in buffer
 * @details returns flag 'isTerminating' from buffer, indicating initiation of program termination\n
 *          global variables used:\n
 *          's_mutex', 'buffer'
 * @return termination flag
 */
int isTerminating(void);

#endif //UE_1B_CIRCULAR_BUFFER_H
