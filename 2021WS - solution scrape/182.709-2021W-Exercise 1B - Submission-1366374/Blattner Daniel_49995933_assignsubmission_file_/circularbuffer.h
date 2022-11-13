/**
 * @file circularbuffer.h
 * @author Daniel Blattner <e12020646@students.tuwien.ac.at>
 * @date 09.11.2021
 *
 * @brief Provides function for an circular buffer for programs.
 *
 * The cicular buffer module create/open shared memory and semaphores for a
 * cirular buffer. The users can send or recieve EdgeArray. If the buffer is
 * full, the senders will wait until a slot was read and therefore cleared.
 * Furthermore the module provides some basic error and debug message 
 * function to stderr and stdout respectively.
**/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <semaphore.h>
#include <signal.h>

#ifndef CIRCULARBUFFER_H__ /* prevent multiple inclusion */
#define CIRCULARBUFFER_H__

/** Name of the shared memory **/
#define SHM_NAME "/12020646.fb_arc_set_shm"

/** Name of the read ready semaphore **/
#define SEM_READRDY "/12020646.fb_arc_set_readrdy"

/** Name of the write ready semaphore **/
#define SEM_WRITERDY "/12020646.fb_arc_set_writerdy"

/** Name of the write mutex semaphore **/
#define SEM_WRITEMUTEX "/12020646.fb_arc.set_writemutex"

/** Number of the slots in the circular buffer **/
#define MAX_DATA 60

/** Maximum number of the edges per slot **/
#define DATA_LIMIT 8

/* 
   MAX_DATA 40 
   DATA_LIMIT 12 
   shared memory allocate 4012 Bytes 
   MAX_DATA 60
   DATA_LIMIT 8 
   shared memory allocate 4092 Bytes 
*/

/** 
 * @struct Edge
 * @brief This struct is an edge in a directional graph defined by its
 * start node and its end node. The id of the nodes have to be positive.
**/
typedef struct{
    /** Id of the start node **/
    int start;
    /** Id of the end node **/
    int end;
}Edge;

/**
 * @struct Graph
 * @brief This struct is a directional graph. It contains an array of
 * directional edges and a list of the nodes, which are present in the
 * edge array. 
**/
typedef struct{
    /** Array of the edges in the graph **/
    Edge *edgeArray;
    /** Lenght of the edge array **/
    unsigned int edgeLen;
    /** Array of the nodes in the graph **/
    int *nodeArray;
    /** Lenght of the node array **/
    unsigned int nodeLen;
}Graph;

/**
 * @struct EdgeArray
 * @brief This struct consist of an edge array with a fixed lenght. It
 * is used as a slot for the shared memory. Therefore it is not possible
 * to send an array with a bigger lenght. 
**/
typedef struct{
    /** Array of edges with a maximum lenght of DATA_LIMIT **/
    Edge array[DATA_LIMIT];
    /** Lenght of the array **/
    unsigned int len;
}EdgeArray;

/**
 * @struct CircularBuffer
 * @brief This struct contains everything that is needed for an 
 * circular buffer. The slots are EdgeArray and limited by the number
 * MAX_DATA. The read/write index point to the next element that can
 * be read/written. In the circular buffer is an interger , which indicate
 * if the generator should terminate. 
**/
typedef volatile struct{
    /** Slots of the circular buffer **/
    EdgeArray data[MAX_DATA];
    /** Index of the next read element **/
    unsigned int readInd;
    /** Index of the next write element **/
    unsigned int writeInd;
    /** Signals the generators to terminate **/
    volatile sig_atomic_t termGen;
}CircularBuffer;

/**
 * @struct SharedResource
 * @brief This struct has every shared resource needed for the communication.
 * This include the circular buffer, the read/write semaphore and the
 * write mutex. 
**/
typedef struct{
    /** Pointer to circular buffer **/
    CircularBuffer *buffer;
    /** Semaphore of read ready **/
    sem_t *readrdy;
    /** Semaphore of write ready **/
    sem_t *writerdy;
    /** Write mutex **/
    sem_t *writemutex;
}SharedResource;

/**
 * @brief Create/Openn the shared memory and map it to the proper size.
 * The shared memory can written and read. After that the semaphores are
 * created/opened. All pointers will be stored in the variable shr.
 * @details If an error occures, the function will close and unmap all
 * semaphores and shared memory. Then it will exit the programm with an
 * EXIT_FAILURE.
 * @param progName The name of the programm as defined in argv[0].
 * @param shr Pointer to the shared resource struct.
 * @param isCreator Indicates if the function create or open 
 * shared memory and semaphores. 
**/
void initSharedResource(char *progName, SharedResource *shr, bool isCreator);

/**
 * @brief Closes/Unmap all resources in the variable shr. If isCreator is
 * true, it additionally unlink the shared memory and semaphores. 
 * @details If an error occures, the function will close and unmap all
 * semaphores and shared memory. Then it will exit the programm with an
 * EXIT_FAILURE. 
 * @param progName The name of the programm as defined in argv[0].
 * @param shr Pointer to the shared resource struct.
 * @param isCreator Indicates if the function should unlink the 
 * shared memory and semphores too.
**/
void destroySharedResource(char *progName, SharedResource *shr, bool isCreator);

/**
 * @brief Checks if a slot is free to write a new data set. If the circular
 * buffer is full, the function waits until a slot gets free. The function
 * copy the content of writeData to the slot and move the write index to the
 * next free slot.
 * @details If an error with the semaphores happend, the function reverse the
 * write index to the original position and prints an error message.
 * @param progName The name of the programm as defined in argv[0].
 * @param shr Pointer to the shared resource struct.
 * @param writeData The edge array which should be written to the 
 * circular buffer.
 * @return Returns true if no error occured, else false. 
**/
bool writeEdgeArray(char *progName, SharedResource *shr, EdgeArray writeData);

/**
 * @brief Checks if a new data set can be read. If no new data set is in the
 * circular buffer, the function wait until otherwise. The content of the slot
 * will be copied to the variable readData. Then the read index will be moved
 * to the next data set. 
 * @details If an error with the semaphores happend, the function reverse the
 * read index to the original position and prints an error message.
 * @param progName The name of the programm as defined in argv[0].
 * @param shr Pointer to the shared resource struct.
 * @param readData The pointer, where the read data should be copied to.
 * @return Returns true if no error occured, else false. 
**/
bool readEdgeArray(char *progName, SharedResource *shr, EdgeArray *readData);

/**
 * @brief Prints the programm name, a custom message (defined in msg) and the
 * content of errno to stderr.
 * @param progName The name of the programm as defined in argv[0].
 * @param msg Specialised message to print.
**/
void printErrorMsg(char *progName, char *msg);

/**
 * @brief Prints the programm name, a custom message (defined in msg) and the
 * content of errno to stderr. Then the programm exits with a EXIT_FAILURE.
 * @param progName The name of the programm as defined in argv[0].
 * @param msg Specialised message to print.
**/
void printErrorMsgExit(char *progName, char *msg);

/**
 * @brief Prints the programm name and a custom message (defined in msg)
 * to stdout.
 * @param progName The name of the programm as defined in argv[0]
 * @param msg Specialised message to print.
**/
void printDebugMsg(char *progName, char *msg);

#endif /* CIRCULARBUFFER_H__ */