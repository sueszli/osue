/**
 * @file myheader.h
 * @author Berke Akkaya 11904656
 * @brief This header file contains some of the includes and structs which are used by the generator and supervisor class
 * 
 * @date 14.11.2021
 * 
 */
#ifndef MYHEADER_H
#define MYHEADER_H

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define SHM_NAME "/11904656_myshm"
#define SEM_FREE_NAME "/11904656_free"
#define SEM_USED_NAME "/11904656_used"
#define SEM_MUTEX_NAME "/11904656_mutex"
#define MAX_RESULT_EDGES 8
#define BUFFER_SIZE 20

typedef struct edges
{
    int startedge;
    int endedge;
} edge;

typedef struct solution
{
    int amount;
    edge edgesolution[MAX_RESULT_EDGES];
} solutions;

typedef struct myshm
{
    unsigned int state;
    int write_pos;
    solutions buffersolution[BUFFER_SIZE];
    int writepos;
    int readpos;
} myshms;

#endif
