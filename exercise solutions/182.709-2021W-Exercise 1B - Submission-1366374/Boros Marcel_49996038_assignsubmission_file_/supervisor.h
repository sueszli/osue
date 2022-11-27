/**
 * @file supervisor.h
 * @author Marcel Boros e11937377@student.tuwien.ac.at
 * @date 12.11.2021
 *
 * @brief Main generator module.
 * 
 * This module contains functions needed for the supervisor to read from the shared memory and synchronize
 * with the generator
 **/



#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>

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

//create shared memory
int createSharedMemory();

//map shared memory
struct myshm* mapSharedMemory(int fd);

//close shared memory
void closeSharedMemory(struct myshm* myshm, int fd, sem_t* free_sem, sem_t* mutex_sem, sem_t* used_sem);

//initialize shared memory
void initialize(struct myshm* myshm);

//read from shared memory
void readSHM(struct myshm* myshm, sem_t* free_sem, sem_t* mutex_sem, sem_t* used_sem);

//main program
int main(int argc, char**argv);

//signal handler (SIGINT, SIGTERM)
void signal_handler(int signal);

#endif