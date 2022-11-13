/** 
 * @file supervisor.c
 * @author Andreas Huber 11809629
 * @date 10.11.2021
 *
 * @brief Supervisor program to read possible solutions from the circular buffer
 * @details supervisor reads the buffer and outputs a new solution every time a
 * a better solution has been found (with lesser edges removed). The supervisor stops
 * either when a acyclic graph has been found or a signal for termination is received
 **/

#include "circularBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

volatile sig_atomic_t quit = 0;

void handle_signal(int signal) {quit = 1;}

/**
 * Program entry point.
 * @brief Program starts here. main function takes care of the parameters and reads the
 * circular buffer until it finds an acyclic graph or gets terminated by a signal
 * @details 
 *          This function uses the following global variables:
 *          ProgramName: stores the Programname from argv[0]
 *          shmem: shared memory object that represents the circular buffer
 *          currentMin: current best solution for the given graph
 *          data: entry from the circular buffer that has ben written by a generator
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_FAILURE if the function gets called wrong, opening of the circular buffer
 * failed or closing of the circular buffer faild * 
 * @return Returns EXIT_SUCCESS otherwise
**/
int main(int argc, char* argv[]){

    //check if called right
    const char* ProgramName = argv[0];
    if(argc>1){
        printf(stderr, "[%s] ERROR: only 1 argument expected");
        exit(EXIT_FAILURE);
    }

    //signal handling
    struct sigaction sa = { .sa_handler = handle_signal};
    sigaction(SIGINT, &sa, NULL);

    //open circular buffer
    sharedMemory* shmem = supervisorOpenCircularBuffer();
    if(shmem == NULL){
        printf(stderr, "[%s] ERROR: supervisorOpenCircularBuffer failed: %s\n", ProgramName, strerror(errno));
        exit(EXIT_FAILURE);    
    }

    /*read circular buffer until termination signal is received or an acyclic
      graph is found */
    int currentMin = 9;
    char* data;
    while(!quit){
        if((data = readCircularBuffer(shmem))==NULL){
            break;
        }
        int count = 0;
        for(int i = 0; data[i] != '\0';i++){
            if(data[i]==' ' && data[i+1]!=' '){
                count++;
            }
        }
        if(count==0){
            fprintf("[%s] The graph is acyclic!", ProgramName);
            free(data);
            break;
        }
        if(count<currentMin){
            currentMin = count;
            fprintf("[%s] Solution with %d edges: %s", ProgramName, count, data);
            free(data);
        }
    }

    //close circular buffer
    if(supervisorCloseCircularBuffer(shmem)==-1){
        printf(stderr, "[%s] ERROR: supervisorCloseCircularBuffer failed: %s\n", ProgramName, strerror(errno));
        exit(EXIT_FAILURE);    
    }
    exit(EXIT_SUCCESS);
}