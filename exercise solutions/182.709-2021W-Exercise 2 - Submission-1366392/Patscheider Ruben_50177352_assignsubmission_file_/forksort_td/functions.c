/**
*   Author: @author Ruben Patscheider
*   MatrikelNummer: 01627951
*   Date: @date 11.12.2021
*   Name of Project: forksort
*   Name of module: functions
*   Purpose of module: 
*   @brief functions.c contains certain functions to make the code more readable
*
*   @details details to each function are provided in the declaration in header.h
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "header.h"


void handleInput(char*** buffer, int* linecount, FILE* file){

    if(file == NULL){
        fprintf(stderr, "Error:[PID: %d][PPID: %d]: File is NULL\n", getpid(), getppid());
    }
    
    size_t len = 0;
    ssize_t nread;
    char *line = NULL;
    char** tmpbuffer;
    int size = INCREMENT;
    size_t memory = sizeof(char*) * size;
    tmpbuffer = (char**)malloc(memory);
    int count = 0;

    while((nread = getline(&line, &len, file)) != -1){
        if(size <= count){
            memory += INCREMENT * sizeof(char*);
            size += INCREMENT;
            tmpbuffer = realloc(tmpbuffer, memory); 
        } 
        tmpbuffer[count] = line;
        count++;
        line = NULL;
    }

    *linecount = count;
    *buffer = tmpbuffer;
    free(line);
}

void execProcess(void){ 
    execlp("./forksort", "forksort", NULL);
    fprintf(stderr, "Error [PID: %d],[PPID %d]: Executing failed: %s\n", getpid(), getppid(), strerror(errno));
    exit(EXIT_FAILURE);
}

void merge(char **child1, char **child2, int child1len, int child2len, char** sorted, int len){

    int i = 0, j = 0, k = 0; 

    while(i < child1len && j < child2len){
        int d = strcmp(child1[i], child2[j]);
        if(d <= 0){
            sorted[k++] = child1[i++];
        }else{
            sorted[k++] = child2[j++];
        }
    }

    while(i < child1len){
        sorted[k++] = child1[i++];
    }
    while(j < child2len){
        sorted[k++] = child2[j++];
    }
}

void freeArray(char** array, int n){
    if(array == NULL){
        return;
    }
    for(int i = 0; i < n; i++){
        free(array[i]);
    }
    free(array);
}

void closePipe(int pipe[2]){
    if(pipe == NULL)
        return;

    close(pipe[POS_READ]);
    close(pipe[POS_WRITE]);
}