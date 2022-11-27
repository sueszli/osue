/*
@autor: Jessl Lukas 01604985
@modulename: generator.c
@created 05.11.2021

This programm get's a list of edges. It writes a list of edges to a shared memory, that create an acylic graph. Those edges are the edges, that would need to be removed from our original graph so that the original graph is 
acyclic. The list of edges written to our shared memory can be empty which means that the original graph is already acyclic without removing any edge. */

#ifndef GENERATOR_H
#define GENERATOR_H

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>


#define CIRCULAR_BUFFER_SIZE 32   //Storage is equal to 2 Kibibyte
#define LENGTH_PER_EDGE 64    // 3 signs + "-" + 3 signs + " "

 struct shm_circular_buffer {
			   int read_position;
               int write_position;
			   int best_length;
               int generator_stop;
               char  circular_buffer[CIRCULAR_BUFFER_SIZE][LENGTH_PER_EDGE];   	// 8 * 8 = 64 byte, 30* 64 = 2048 bytes, so we are still within our max range of 2048	
           };


char* unsortedinputnodes(char* wedges, char* graphnodes);
char* removedoublenodes (char* graphnodes, int* p_numberofnodes);
char* randomizenodes(char* graphnodes, int numberofnodes);
char* testwedges(char* randomizednodes, char* wedges, int numberofnodes, int* p_Edges);
void closesharedmemory(struct shm_circular_buffer *shm_cm, int fd, char* shm_name, sem_t* mutex, sem_t* sem_read_left_1604985, sem_t* sem_write_left_1604985);



#endif
