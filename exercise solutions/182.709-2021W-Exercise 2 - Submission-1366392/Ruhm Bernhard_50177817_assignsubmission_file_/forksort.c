/**
*
*@file forksort.c
*@author Bernhard Ruhm
*@date last edit 12-11-2021
*@brief program that sorts lines alphabetically
*@details This program represent a variation of mergesort. It reads lines from stdin and writes 
*the sorted lines to stdout. Each branch of the mergesort algorithm is realized by calling this 
*program recursivly. Therore each branch represents one partial solution. The programm terminates
*successfully, if only 1 line was read from stdin or after the sorted lines were written to stdout
*
*
**/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

const char *prog;

//struct to store pid and fd of pipes from child
typedef struct child{
    int pid;
    int write_pipe[2];
    int read_pipe[2];
}child;

/**
*usage function
*@brief writes an usage message to stderr and the program exits
*on EXIT_FAILURE
*
**/
static void usage(){
    fprintf(stderr, "[%s] usage: %s\n", prog, prog);
    exit(EXIT_FAILURE);
}


/**
*print error message and exit programm
*@brief writes an error message to stderr and exits the program
*@details this function writes a given error message and the appropriate errno code message
*to stderr
*@param msg message printed to stderr
*@param e errno code printed to stderr
*
**/
static void error_exit(char *msg, int e){
    fprintf(stderr, "%s: %d\n",msg, e);
    exit(EXIT_FAILURE);
}

/**
*create a child
*@brief forks a child of this programm
*@details this function creates a child of this programm by
*calling fork() and exec(). The communication between the parent and 
*child process is realized by using unnamed pipes.
*@param child child to store pid and pipe fds
*
**/
static void fork_child(child *child ){

    if((pipe(child->write_pipe) == -1) || (pipe(child->read_pipe) == -1)){
        error_exit("creating pipe failed", errno);
    }
    
    switch((child->pid = fork())){
        case -1:
            error_exit("fork failed", errno);
            break;
        case 0:
        //child
            //dup write end
            close(child->write_pipe[0]);
            if(dup2(child->write_pipe[1], STDOUT_FILENO) == -1)
                error_exit("duplicate write end failed", errno);
            close(child->write_pipe[1]);

            //dup read end
            close(child->read_pipe[1]);
            if(dup2(child->read_pipe[0], STDIN_FILENO) == -1)
                error_exit("duplicate read end failed", errno);
            close(child->read_pipe[0]);

            if(execlp(prog, prog, NULL) == -1)
                error_exit("exec failed", errno);
        default:
        //parent
            close(child->write_pipe[1]);
            close(child->read_pipe[0]);
            break;
    }
}

/**
*sort lines alphabetically
*@brief sorts the given solution alphabetically (case sensitiv)
*@details this function sorts the two line arrays alphabetically and writes
*the sorted line array to stdout
*@param sol1 first line array 
*@param size1 size of first line array
*@param sol2 second line array
*@param size2 size of second line array
*
**/
static void mergesort(char ***sol1, int size1, char ***sol2, int size2){

    int i1 = 0;
    int i2 = 0;
    int cmp_size = 0;

    while((i1 < (size1)) && (i2 < (size2))){
        //size of smaller input is the size to compare
        if(strlen((*sol1)[i1]) > strlen((*sol2)[i2]))
            cmp_size = strlen((*sol2)[i2]);
        else
            cmp_size = strlen((*sol1)[i1]);
        //"smaller" line is written to stdout
        if(strncmp((*sol1)[i1], (*sol2)[i2], cmp_size) < 0){
            fprintf(stdout, "%s", (*sol1)[i1]);
            i1++;
        }
        else{
            fprintf(stdout, "%s", (*sol2)[i2]);
            i2++;
        }
    }
    //write remaining lines to stdout
    if(i2 == size2){
        for(;i1 < size1; i1++){
            fprintf(stdout, "%s", (*sol1)[i1]);
        }
    }
    else{
        for(;i2 < size2; i2++){
            fprintf(stdout, "%s", (*sol2)[i2]);
        }
    }
    fflush(stdout);
}

/**
*read and merge solution
*@brief reads lines from children and merges solutions
*@details this function reads 1 sorted line array from each child process.
*These two solutions are forged into one by calling mergesort()
*@param c1 file pointer of child 1
*@param c2 file pointer of child 2
*@return
*
**/
static void read_and_merge(FILE *c1, FILE *c2){

    char **c1_solution = malloc(sizeof(char *));
    char **c2_solution = malloc(sizeof(char *));
    int c1_solution_size = 0;
    int c2_solution_size = 0;
    int index = 0;
    size_t l = 0;
    
    char *tmp = NULL;

    //read solution from child 1
    while(getline(&tmp, &l, c1) != -1){
        c1_solution_size++;
        c1_solution = realloc(c1_solution, c1_solution_size * sizeof(char *));
        c1_solution[index] = malloc(strlen(tmp) + 1);
        strncpy(c1_solution[index], tmp, strlen(tmp));
        c1_solution[index][strlen(tmp)] = '\0';
        index++;
    }
    index = 0;
    //read solution from child 2
    while(getline(&tmp, &l, c2) != -1){
        c2_solution_size++;
        c2_solution = realloc(c2_solution, c2_solution_size * sizeof(char *));
        c2_solution[index] = malloc(strlen(tmp) + 1);
        strncpy(c2_solution[index], tmp, strlen(tmp));
        c2_solution[index][strlen(tmp)] = '\0';         //ensure that string ist 0-terminated
        index++;
    }

    //sort the 2 solutions
    mergesort(&c1_solution, c1_solution_size, &c2_solution, c2_solution_size);

    for(int i = 0; i < c1_solution_size; i++){
        free(c1_solution[i]);
    }
    for(int i = 0; i < c2_solution_size; i++){
        free(c2_solution[i]);
    }
    free(c1_solution);
    free(c2_solution);
    free(tmp);
}


/**
*main function
*@brief reads lines from stdin and sorts them alphabetically
*@details The program starts here. The main function provides the main 
*functionality of this program, by calling the functions fork_child(),
*read_and_merger(), mergesort().
*global variables: prog
*@param argc argument count
*@param argv argument vector
*@return EXIT_SUCCESS on successfull termination
*        EXIT_FAILURE on failed termination
*
**/
int main(int argc, char **argv){

    prog = argv[0];

    char *line1 = NULL;
    char *line2 = NULL;

    size_t length1 = 0;
    size_t length2 = 0;

    if(argc != 1)
        usage();
    
    if((getline(&line1, &length1, stdin)) == -1){
    //wrong input
        usage();
    }

    if((getline(&line2, &length2, stdin)) == -1){
    //only 1 line as input, child terminates
        fprintf(stdout, "%s", line1);
        fflush(stdout);
        free(line1);
        free(line2);
        exit(EXIT_SUCCESS); 
    }

//at least 2 lines as input
    int status = 0;
    child child1;
    child child2;

    fork_child(&child1);
    fork_child(&child2);
    
//parent
    FILE *c1_write = fdopen(child1.read_pipe[1], "w");
    FILE *c1_read = fdopen(child1.write_pipe[0], "r");
    FILE *c2_write = fdopen(child2.read_pipe[1], "w");
    FILE *c2_read = fdopen(child2.write_pipe[0], "r");

    fprintf(c1_write, "%s", line1);
    fprintf(c2_write, "%s", line2);
  
    //read and write remaining lines to children
    while (1)
    {
        if((getline(&line1, &length1, stdin)) == -1)
            break;
        fprintf(c1_write, "%s", line1);

        if((getline(&line2, &length2, stdin)) == -1)
            break;
        fprintf(c2_write, "%s", line2);
    }
    
    fflush(c1_write);
    fflush(c2_write);
    close(child1.read_pipe[1]);
    close(child2.read_pipe[1]);
    fclose(c1_write);
    fclose(c2_write);
    
    waitpid(child1.pid, &status, 0);
    waitpid(child2.pid, &status, 0);

    read_and_merge(c1_read, c2_read);

    fclose(c1_read);
    fclose(c2_read);
    close(child1.write_pipe[0]);
    close(child1.read_pipe[0]);

    free(line1);
    free(line2);

    return EXIT_SUCCESS;
}

    



     
