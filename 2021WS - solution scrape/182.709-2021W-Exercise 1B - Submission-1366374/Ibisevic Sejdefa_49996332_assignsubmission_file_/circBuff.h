/**
 * @file circBuff.h
 * @author Sejdefa Ibisevic <e11913116@student.tuwien.aca.at>
 * @brief The circBuff.h file represents h file for shared memory for helping execution of generators and supervisor
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */


#ifndef CIRCBUFF_H
#define CIRCBUFF_H


#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h> /* For O_* constants */

//SHM constants
#define SHM_NAME 	"/11913116SHM"

//SEM constants
#define FREE_SEM 	"/11913116_free"
#define USED_SEM  	"/11913116_used"
#define MUTEX_SEM	"/11913116_mutex"

#define BUFFER_SIZE	64
//Solutions with more then 8 edges not accepted
#define MAX_EDGE 	8
#define SHM_MAX_DATA (1024)

#define ERROR_MSG(...) { fprintf(stderr, "ERROR: " __VA_ARGS__"\n"); }

// Struct edge in form A_node-B_node
typedef struct Edge{
  int Anode;
  int Bnode;
} edge;

//Struct for a buffer
/**
 * @brief Struct for the buffer which is used for shared memory
 * 
 * @details
 * buffer - holds edge data
 * pos_read - holds position of read
 * pos_write - holds write position
 * exit - flag if supervisoer has finished and all generators should exit
 */
typedef struct buffer {

  char circular_buffer[SHM_MAX_DATA];	
	int pos_write; 		//Position to read from
	int pos_read;			//Position to write to
  bool exit; 
  
} buffer;

buffer *open_buff (int type);
int write_buff (buffer *buff, const char *data);
char *read_buff(buffer *buff);
int close_buff(buffer *buff, int type);

#endif
