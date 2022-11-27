/**
*@file supervisor.h
*@author Jasmine Haider <e11943664@student.tuwien.ac.at>
*@date 14.11.2021
*
*@brief supervisor program module
*
* This program creates shared memory and semaphores for one or multiple generators to store solutions for the
* 3-color problem in a circular buffer. It reads solutions from the buffer, storing the best solution that has
* been found so far and prints to stdout if a new solution has fewer edges than the last stored solution.
*
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include <regex.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//10 entries in circular buffer
#define CIRCBUF_SIZE 10
//number of edges allowed in solution
#define SOLUTION_SIZE 8

#define SHARED_MEMORY "/11943664_graph"
#define SEM_BUFFER_ACCESS "/11943664_sem_buffer_access"
#define SEM_BUFFER_FREE "/11943664_sem_buffer_free"
#define SEM_BUFFER_USED "/11943664_sem_buffer_used"

struct edge
{
  int vertex1;
  int vertex2;
};

struct solution
{
  struct edge edges[SOLUTION_SIZE];
  int number;			//number of edges that should be deleted
};

struct shared_struct
{
  int terminate;
  int writeindex;
  int readindex;
  int best_solution_edges;
  int generator_count;
  struct solution solutions[CIRCBUF_SIZE];
};
