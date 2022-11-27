/**
*   Author: @author Ruben Patscheider
*   MatrikelNummer: 01627951
*   Date: @date 11.12.2021
*   Name of Project: forksort
*   Name of module: forksort
*   Purpose of module: 
*   @brief by forking until each child only has one element left, an input with
*   random size will be sorted using a forked mergesort algorithm
*
*   @details after handling the input given through stdin, the program spilts the 
*   amount of lines and forkes twice. Each child opens a new forksort program
*   and recevies as input half of the input that the root program received, through 
*   a pipe that functions as redirected stdin.
*   This happens recursively until each child only has 1 element left and writes it 
*   back through a redirected stdout pipe. 
*   The parent will then compare each subjection of elements and merg them to create
*   an alphabetically sorted output.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <getopt.h>

#include "header.h"


int main(int argc, char *argv[]){

///< Argument Handling
    char c;
    while((c = getopt(argc, argv, "")) != -1){
        switch(c){
            case '?': 
                fprintf(stderr, "Error in %s[PID: %d][PPID: %d]: the program does not take any arguments\n", argv[0], getpid(), getppid());
            default:
                exit(EXIT_FAILURE);
        }
    }


///< Input Handling
    char **buffer; ///< buffer array where the input from stdin is stored
    int linecount; ///< number of lines read from input will be stored here

    handleInput(&buffer, &linecount, stdin);

    if(linecount == 0){ ///< If linecount is 0 the program is finished, if it is 1, the only element is written into stdout.
        freeArray(buffer, linecount);
        return EXIT_SUCCESS;
    }else if(linecount == 1){
        fputs(buffer[0], stdout);
        freeArray(buffer, linecount);
        return EXIT_SUCCESS;
    }

///< Spliting input 
    ///< generating two separat arrays for each kid
    char** child1array = (char**)malloc((linecount/2) * sizeof(char*));
    char** child2array;
    int child2_len; ///< if input lenght is odd, child2 gets an odd number of elements, that number is saved here
    if(linecount%2 == 0){
        child2_len = linecount/2;
    }else{
        child2_len = (linecount/2)+1;
    }

    child2array = (char**)malloc(child2_len * sizeof(char*));

    for(int i = 0, k = 0; i < linecount; i++){
        int tmp = strlen(buffer[i]) + 1;
        if(i < (linecount/2)){
            child1array[i] = (char*)malloc(tmp * sizeof(char));
            strcpy(child1array[i], buffer[i]);
        }else{
            child2array[k] = (char*)malloc(tmp * sizeof(char));
            strcpy(child2array[k++], buffer[i]);
        }
    }

    freeArray(buffer, linecount);

//________________________________________Child1_____________________________________________//
   
///< Generating pipes
    int child1_result[2]; ///< fd for pipe where child writes its solution to parent
    int child1_input[2]; ///< fd for pipe where parent writes to child, input data 

///< check if pipes where opened correctly, if not an error is printed and everything is freed
    if(pipe(child1_result) == -1){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: Opening write_child1 pipe failed: %s", argv[0], getpid(), getppid(), strerror(errno));
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        exit(EXIT_FAILURE);
    }
    if(pipe(child1_input) == -1){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: Opening read_child1 pipe failed: %s", argv[0], getpid(), getppid(), strerror(errno));
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        exit(EXIT_FAILURE);
    }

///writing to child1 
    FILE* file_child1 = fdopen(child1_input[POS_WRITE], "w"); ///< parent opens the input pipe as a file and writes data for the child to process
    if(file_child1 == NULL){
        fprintf(stderr, "Error:[PID: %d][PPID: %d]: Opening file_child1 failed\n", getpid(), getppid());
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < linecount/2; i++){
        fputs(child1array[i], file_child1);
    }
    fclose(file_child1);
    close(child1_input[POS_WRITE]);

    pid_t pid_1 = fork();///< parent forks the first time and checks if the fork was done successfully
    if(pid_1 == -1){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: fork pid1 failed: %s", argv[0], getpid(), getppid(), strerror(errno));
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        exit(EXIT_FAILURE);
    }
  
    if(pid_1 == 0){ ///< code only done by child1

        close(child1_result[POS_READ]); ///< closing of unneeded ends
        dup2(child1_result[POS_WRITE], STDOUT_FILENO); ///< redirecting of stdout to the write end of the result pipe
        close(child1_input[POS_WRITE]);
        dup2(child1_input[POS_READ], STDIN_FILENO);///< redirecting of stdin from child1 to read position of input pipe
        if(linecount > 1){
            execProcess();
        }
    }

    close(child1_result[POS_WRITE]);

//________________________________________Child2_____________________________________________//


///< This section copies the Child1 section exactly.
///< Generating pipes
    int child2_result[2];
    int child2_input[2];
    
    if(pipe(child2_result) == -1){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: Opening write_child2 pipe failed: %s", argv[0], getpid(), getppid(), strerror(errno));
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        exit(EXIT_FAILURE);
    }
    if(pipe(child2_input) == -1){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: Opening read_child2 pipe failed: %s", argv[0], getpid(), getppid(), strerror(errno));
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        closePipe(child2_result);
        exit(EXIT_FAILURE);
    }

///<write to child2
    FILE* file_child2 = fdopen(child2_input[POS_WRITE], "w");
    if(file_child2 == NULL){
        fprintf(stderr, "Error:[PID: %d][PPID: %d]: Opening file_child2 failed\n", getpid(), getppid());
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        closePipe(child2_result);
        closePipe(child2_input);
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < (child2_len); i++){
        fputs(child2array[i], file_child2);
    }

    fclose(file_child2);
    close(child2_input[POS_WRITE]);

///< Process generation
    pid_t pid_2 = fork();
    if(pid_2 == -1){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: fork pid2 failed: %s", argv[0], getpid(), getppid(), strerror(errno));
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        closePipe(child2_result);
        closePipe(child2_input);
        exit(EXIT_FAILURE);
    }

    if(pid_2 == 0){
        close(child2_result[POS_READ]);
        dup2(child2_result[POS_WRITE], STDOUT_FILENO);
        close(child2_input[POS_WRITE]);
        dup2(child2_input[POS_READ], STDIN_FILENO);
        if(linecount > 1){
            execProcess();
        }
    }

    close(child2_result[POS_WRITE]);

//________________________________________Continuation of parent code_____________________________________________//

///< waiting
    int child1_status; ///< saves the return value of exit of child1
    int child2_status; ///< saves the return value of exit of child2
    waitpid(pid_1, &child1_status, 0); ///< waiting til child1 finishes
    waitpid(pid_2, &child2_status, 0); ///< waiting til child2 finishes
    ///< if child1 or child2 do not exit with success, an error is generated
    if(WEXITSTATUS(child1_status) != EXIT_SUCCESS){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: child1 did not exit properly\n", argv[0], getpid(), getppid());
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        closePipe(child2_result);
        closePipe(child2_input);
        exit(EXIT_FAILURE);
    }

    if(WEXITSTATUS(child2_status) != EXIT_SUCCESS){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: child2 did not exit properly\n", argv[0], getpid(), getppid());
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        closePipe(child2_result);
        closePipe(child2_input);
        exit(EXIT_FAILURE);
    }

///<reading from children
    char** readchild1array; ///< array to save what child1 wrote into the result pipe
    int readchild1count; ///< nuumber of elements read
    FILE* file_child1_input = fdopen(child1_result[POS_READ], "r");
    if(file_child1_input == NULL){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: Opening read file from child1 failed\n", argv[0], getpid(), getppid());
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        closePipe(child2_result);
        closePipe(child2_input);
        exit(EXIT_FAILURE);
    }
    handleInput(&readchild1array, &readchild1count, file_child1_input);
    fclose(file_child1_input);

    char** readchild2array; 
    int readchild2count;
    FILE* file_child2_input = fdopen(child2_result[POS_READ], "r");
    if(file_child1_input == NULL){
        fprintf(stderr, "Error in %s, [PID: %d][PPID: %d]: Opening read file from child1 failed\n", argv[0], getpid(), getppid());
        freeArray(child2array, child2_len);
        freeArray(child1array, (linecount/2));
        closePipe(child1_result);
        closePipe(child1_input);
        closePipe(child2_result);
        closePipe(child2_input);
        exit(EXIT_FAILURE);
    }
    handleInput(&readchild2array, &readchild2count, file_child2_input);
    fclose(file_child2_input);

///< sorting and merging
    int readchildcount = readchild1count + readchild2count; ///< number of elements from both subarrays

    char **sorted_list = (char**)malloc(readchildcount * sizeof(char *)); ///< result array that stores sorted output

    merge(readchild1array, readchild2array, readchild1count, readchild2count, sorted_list, readchildcount);
  
///< result from merge() is saved in sorted_list and written into the stdout
    for(int i = 0; i < readchildcount; i++){
        fprintf(stdout, "%s", sorted_list[i]);
    }


///< resources are freed/closed
    freeArray(readchild1array, readchild1count);
    freeArray(readchild2array, readchild2count);
    free(sorted_list);
    freeArray(child2array, child2_len);
    freeArray(child1array, (linecount/2));
    closePipe(child1_result);
    closePipe(child1_input);
    closePipe(child2_result);
    closePipe(child2_input);
    return EXIT_SUCCESS;
}