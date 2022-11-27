/*
*
* @file shared_Memory.h
* @author Jakob Frank (11837319)
* 
* @brief defines basic structures and types
*
* @date 12/11/2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#define SHM_NAME "/myshm"
//MAX_DATA defines the maximum of edges within an instance of shm
#define MAX_DATA (8)

typedef struct edge
{
int start;
int end;
} edge;

typedef struct myshm
{   
    edge edges[MAX_DATA];
    int size;
} shm_t;

