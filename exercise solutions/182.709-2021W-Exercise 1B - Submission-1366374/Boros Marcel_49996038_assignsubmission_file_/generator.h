/**
 * @file generator.h
 * @author Marcel Boros e11937377@student.tuwien.ac.at
 * @date 12.11.2021
 *
 * @brief Main generator module.
 * 
 * This module contains functions needed for the generator to read/write from the shared memory and synchronize
 * with the supervisor.
 **/


#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>

#define CIRCULAR_BUFFER_SIZE 2048
#define MAX_LENGTH 64 // 8*sizeof(char)*3 + 8; 8 edges of the form "a-b" and the spaces between them + "\0"
#define ENTRIES CIRCULAR_BUFFER_SIZE/MAX_LENGTH
#define SHM_NAME "11937377_shm"
#define S1 "11937377_free_space_sem"
#define S2 "11937377_used_space_sem"
#define S3 "11937377_mutex"


struct myshm {
    int terminate;
    int bestSolution_length;
    int readEnd;
    int writeEnd;
    char bestSolution[MAX_LENGTH];  
    char data[ENTRIES][MAX_LENGTH];
};

//Create random permutation of nodes
void FisherYates(char* nodes);

//Read-in edges from command line (positional arguments)
void readEdges(char** edges, int argc, char** argv);

//Create a node list from the edges
void createNodes(char** edges, char* nodes, int argc);

//Copy string array
void cpyStrArray(char** a, char** b, int size);

//Select chosen edges
int solution(int argc, char* nodes, char** chosenEdges, char** cpy_edges2, char** edges);

//write solution to shared memory
void writeSHM(struct myshm* myshm, char** chosenEdges, int size, sem_t* free_sem, sem_t* mutex_sem, sem_t* used_sem);

//open shared memory
int openSharedMemory();

//map shared memory to adress
struct myshm* mapAddress(int fd);


//close shared memory for generator
void closeSHM_gen(struct myshm* myshm, int fd, sem_t* free_sem, sem_t* mutex_sem, sem_t* used_sem);


//main program
int main(int argc, char**argv);

#endif