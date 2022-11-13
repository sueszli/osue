#ifndef __MYUTIL_H__ 
#define __MYUTIL_H__

/**
 * @file myutil.h
 * @author Bernhard Jung: 12023965
 * @date 2021.11.14
 * @brief Contains utilities for supervisor.c and generator.c
 */

/**
 * @brief Name of Semaphore which indicates how much free space there is in the share memory ringbuffer.
 */
#define SEM_1_FREE "/12023965_mySem_1_free"

/**
 * @brief Name of Semaphore which indicates how much used space there is in the share memory ringbuffer.
 */
#define SEM_2_USED "/12023965_mySem_2_used"

/**
 * @brief Name of Semaphore which helps to indicate how many generator prozesses are running
 */
#define SEM_3_CNT "/12023965_mySem_3_cnt"

/**
 * @brief Name of Semaphore which prevents writing errors on the ringbuffer from happening
 */
#define SEM_4_WRITE "/12023965_mySem_4_write"

/**
 * @brief Name of the shared memory region
 */
#define MEMREGION "/12023965_myShemRegion"

/**
 * @brief Size of ringbuffer
 */
#define RINGBUFFERSIZE 8

/**
 * @brief Max allowed length of a resultin the ringbuffer
 */
#define MAXRESULTLENGTH 62

#include <semaphore.h>

/**
 * @struct generatorGraph_t
 * @brief struct for temporarly storing the graph which is given to the generator process via input arguments
 * Edges:     (edge_from[] - edge_to[]) and num_edges
 * Vertices:  (vertices[]) and num_vertices
 */
typedef struct
{
  int * edge_from;
  int * edge_to;
  int num_edges;
  int * vertices;
  int num_vertices;
} generatorGraph_t;

/**
 * @struct shmRegion_t;
 * @brief Struct for the shared memory region
 * running:       1 if the supervisor is currently running
 * end:           1 if the programs should be terminated
 * process_count: number of generator processes running
 * write_head:    position of the write_head of the ringbuffer
 * result_length: length ob individual results written to the ringbuffer
 * result_from:   ringbuffer for the "from" of resulting edges
 * resukt_to:     ringbuffer for the "to" of resulting edges 
 */
typedef struct {
  int running;
  int end;
  int process_count;
  int write_head;
  int result_length[RINGBUFFERSIZE];
  int result_from[RINGBUFFERSIZE][MAXRESULTLENGTH];
  int result_to[RINGBUFFERSIZE][MAXRESULTLENGTH];
} shmRegion_t;

/**
 * @brief closes semaphore that is given in the arguments; prints error if an error occurs
 * @param sem Graph-struct who's arrays are being freed
 * @param arg argument for displaying possible error messages
 * @return returns 0 is no error has occured; -1 if an error has occured
 */
int mySemClose(sem_t * sem,char * arg);

/**
 * @brief opens semaphore that is given in the arguments; prints error if an error occurs
 * @param sem Graph-struct who's arrays are being freed
 * @param arg argument for displaying possible error messages
 * @return returns 0 is no error has occured; -1 if an error has occured
 */
int mySemOpen(sem_t * sem,char * arg);

/**
 * @brief posts semaphore that is given in the arguments; prints error if an error occurs
 * @param sem Graph-struct who's arrays are being freed
 * @param arg argument for displaying possible error messages
 * @return return value of sem_post(sem); -1 if and error has occured
 */
int mySemPost(sem_t * sem, char * arg);

/**
 * @brief waits semaphore that is given in the arguments; prints error if an error occurs
 * @param sem Graph-struct who's arrays are being freed
 * @param arg argument for displaying possible error messages
 * @return return value of sem_wait(sem); -1 if and error has occured
 */
int mySemWait(sem_t * sem, char * arg);

#endif