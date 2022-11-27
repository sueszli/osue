/**
 * @file forksort.c
 * @author Kevin Wielander <e11908531@student.tuwien.ac.at>
 * @date: 10.12.2021
 * @brief Complete Implementation of the forksort task.
 */
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>


//defined names for the indexes of the pipes for better understanding
#define PARENT_CHILD1 0
#define PARENT_CHILD2 1
#define CHILD1_PARENT 2
#define CHILD2_PARENT 3

// Programname
char *myprog;

/**
 *@brief prints the synopsis of the mydiff command
 *@details terminates with exit_code 0, global variable myprog is used       
 **/
static void usage(void){
    fprintf(stderr, "Usage: %s file1 \n",myprog);
    exit(EXIT_FAILURE);
}

/**
 * error handling
 * @brief prints an errormessage and exits with EXIT_FAILURE
 * @param msg string to be printed in error message
 **/
static void error(char* msg){
    fprintf(stderr, "%s - %s: %s\n", myprog, msg, strerror(errno));
    exit(EXIT_FAILURE);
}
/**
 * Close 2 filedescriptors
 * @brief closes 2 filedescriptors
 * @param fd0 first filedescrriptor
 * @param fd1 second filedescriptor
 **/
static void close2(int fd0, int fd1){
    if(close(fd0) == -1){
        error("fd close failed");
    }
    if(close(fd1) == -1){
        error("fd close failed");
    }
}
/**
 * @brief entry point of the program
 * @details sorts lines from stdin by using fork to recursively 
 * subdivide the lines into half and connects the sorted lines back together
 * inter process communication via pipes sending to stdin of child and reading from stdout from child using dup2 for redirection#
 **/

int main(int argc, char *argv[])
{ 
    myprog = argv[0];

    char* firstLine = NULL;
    char* secondLine = NULL;
    ssize_t lineLen1 = 0;
    ssize_t lineLen2 = 0;
    size_t len;

    if(argc != 1){
        usage();
    }
    //check if parent stdin has at least two lines
    if((lineLen1 = getline(&firstLine,&len,stdin)) == -1 || (lineLen2 = getline(&secondLine,&len,stdin)) == -1){  
        fprintf(stdout,"%s\n",firstLine);    
        exit(EXIT_SUCCESS);
    }

    //open 4 unnamed pipes
    int pipes[4][2];
    if(pipe(pipes[PARENT_CHILD1]) == -1 || pipe(pipes[PARENT_CHILD2]) == -1 || pipe(pipes[CHILD1_PARENT]) == -1 || pipe(pipes[CHILD2_PARENT]) == -1 ){
        error("pipe failed");
    }
    //fork to generate child1
    pid_t pid1 = fork();
    if(pid1 == -1){
        error("fork failed");
    }
    if(pid1 == 0){   // child1 branch
        //redirect read end of Pipe to stdin
        if((dup2(pipes[PARENT_CHILD1][0],STDIN_FILENO)) == -1){
            error("dup2 failed");
        }
        //redirect writeend of pipe to stdout
        if((dup2(pipes[CHILD1_PARENT][1],STDOUT_FILENO)) == -1){
            error("dup2 failed");
        }
        //close all pipes
        for (int i = 0; i < 3; i++)
        {
            close2(pipes[i][0],pipes[i][1]);
        }
        //recursion 
        if(execlp("./forksort",argv[0],NULL) == -1){
            error("exclp failed");
        }    
    }
    else{   // parent branch
        //fork to generate child2
        pid_t pid2 = fork();
        if(pid2 == -1){
            error("fork failed");
        }
        if(pid2 == 0){   //child2 branch
            //redirect read end of Pipe to stdin
            if((dup2(pipes[PARENT_CHILD2][0],STDIN_FILENO)) == -1){
                error("dup2 failed");
            }
            //redirect write end of Pipe to stdout
            if((dup2(pipes[CHILD2_PARENT][1],STDOUT_FILENO)) == -1){
                error("dup2 failed");
            }
            //close all pipes
            for (int i = 0; i < 3; i++)
            {
                close2(pipes[i][0],pipes[i][1]);
            }
            //recursion
            if(execlp("./forksort",argv[0],NULL) == -1){
                error("exclp failed");
            }
        }
        else{   // parent branch 
            //close end of pipes which arent used
            close2(pipes[PARENT_CHILD1][0],pipes[PARENT_CHILD2][0]);
            close2(pipes[CHILD1_PARENT][1],pipes[CHILD2_PARENT][1]);
            int state = 0;
            char *line;
            size_t len = 0;
            ssize_t lineLen;
            int ctr = 2;
            //write the checked lines from line:73 to the children
            write(pipes[PARENT_CHILD1][1],firstLine,lineLen1);   
            write(pipes[PARENT_CHILD2][1],secondLine,lineLen2);   
            //free buffers
            free(firstLine);
            free(secondLine);
            // odd to child1 , even to child2
            while((lineLen = getline(&line,&len,stdin)) != -1){
                if((++ctr) % 2 == 1){
                    // write to stdin of child1
                    write(pipes[PARENT_CHILD1][1],line,lineLen);   
                }
                else{
                    // write to stdin of child2
                    write(pipes[PARENT_CHILD2][1],line,lineLen);    
                }
            }
            free(line);
            close2(pipes[PARENT_CHILD1][1],pipes[PARENT_CHILD2][1]);
            //wait for child1
            waitpid(pid1, &state,0);
            if(WEXITSTATUS(state)){
                error("child1 failed");
            }
            //wait for child2
            waitpid(pid2, &state,0);
            if(WEXITSTATUS(state)){
                error("child2 failed");
            }
            //open pipes as files
            FILE *fileLeft = fdopen(pipes[CHILD1_PARENT][0], "r");
            FILE *fileRight = fdopen(pipes[CHILD2_PARENT][0], "r");
            if(fileLeft == NULL || fileRight == NULL){
                error("fdopen failed!");
            }
            int ctr1, ctr2;
            ctr1 = ctr/2;
            ctr2 = ctr/2;
            if(ctr % 2 == 1){
                ctr1 = (ctr/2)+1;
            }
            
            char *lineLeft = NULL;
            char *lineRight = NULL;
            size_t lenLeft = 0;
            size_t lenRight = 0;
            char **readChild1 = malloc(sizeof(char*) * ctr1);
            char **readChild2 = malloc(sizeof(char*) * ctr2);
            int i = 0;
            while(((getline(&lineLeft,&lenLeft,fileLeft)) != -1) && ((getline(&lineRight,&lenRight,fileRight)) != -1)){
                readChild1[i] = lineLeft;
                readChild2[i] = lineRight;
                i++;
                lineLeft = NULL;
                lineRight = NULL;
            }
            //free(lineLeft);
            free(lineRight);
            if(ctr1 != ctr2){
                readChild1[i] = lineLeft;
            }
            
            int left = 0;
            int right = 0;
            //concat both childs using mergesort algorithmus
            while(ctr1 != left && ctr2 != right){
                if((strcmp(readChild1[left],readChild2[right])) < 0){
                    fprintf(stdout,"%s", readChild1[left]);
                    free(readChild1[left]);
                    left++;
                }
                else{
                    fprintf(stdout,"%s", readChild2[right]);                
                    free(readChild2[right]);
                    right++;
                }
            }
            while(ctr1 != left || ctr2 != right){
                if(ctr1 != left){
                    fprintf(stdout,"%s", readChild1[left]);
                    free(readChild1[left]);
                    left++;
                }
                else{
                    fprintf(stdout,"%s", readChild2[right]);
                    free(readChild2[right]);
                    right++;
                }
            }
            // free remainiung open filedescriptors
            free(fileLeft);
            free(fileRight);
            free(readChild1);
            free(readChild2);
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}
