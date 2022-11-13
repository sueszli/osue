/*
@autor: Jessl Lukas 01604985
@modulename: generator.c
@created 05.11.2021

This programm createsa shared memory, which is being written on from another programm called generator. It then reads the written solutions from that shared memory 
and safes the best solutions (if a solution has less edges it is seen as better). If a soltuion with 0 edges is given by the generator, then the input graph is already acylic
and the supervisor will terminate all generators and exit with success. Otherwhise the programm will run in a loop in the attempt to find a solution with 0 edges. */


#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>


#define CIRCULAR_BUFFER_SIZE 32   //Storage is equal to 2 Kibibyte
#define LENGTH_PER_EDGE 64    // 3 signs + "-" + 3 signs + " "

 struct shm_circular_buffer {
			   int read_position;
               int write_position;
			   int best_length;
               int generator_stop;
               char  circular_buffer[CIRCULAR_BUFFER_SIZE][LENGTH_PER_EDGE];   	// 8 * 8 = 64 byte, 32* 64 = 2048 bytes, so we are still within our max range of 2048	
           };

void signal_handler(int signal);
int createsharedmemory();
struct shm_circular_buffer* mapshm(int fd);
void closesharedmemory( int fd, sem_t* mutex , sem_t * sem_read_left_1604985, sem_t* sem_write_left_1604985);
char* readfromcircularbuffer( char* smallest_solution);


#endif
