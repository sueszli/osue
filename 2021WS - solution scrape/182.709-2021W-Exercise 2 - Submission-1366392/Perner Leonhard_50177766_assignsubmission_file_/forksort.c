/**
 * @file forksort.c
 * @author Leonhard Perner <e12020652@student.tuwien.ac.at>
 * @date 08.12.2021
 *
 * @brief forksort
 * 
 * This program uses the concepts of mergesort and fork
 * to sort the input lines from stdin an print them to stdout
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROG_NAME "forksort"


int myInitPipes(int arr1[2][2], int arr2[2][2]);
void myFork(int *c, int usedPipes[2][2], int unusedPipes[2][2]);
int myClosePipeEnds(int arr[2][2]);
int myDup(int arr[2][2]);
int myMergeSort(FILE *fd1, FILE *fd2);
int myCompPrint (char *l1, char *l2);
void myFree(char *a, char *b);
void myErrorExit(char *msg);

pid_t pid = -1;

/**
 * Program entry point.
 * @brief The program starts here. 
 * @details Handles arguments, calls functions for input, output and forking
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv){
    pid = getpid();
    if (argc != 1)
    {
        myErrorExit("No arguments allowed!");
    }

    char *line = "";
    size_t lSize = 0;
    // checking for input
    if(getline(&line, &lSize, stdin) == -1){
        free(line);
        myErrorExit("No Input!");
    }

    char *temp =  malloc(strlen(line) * sizeof(char));
    strcpy(temp, line);
    // base case
    if(getline(&line, &lSize, stdin) == -1){
        printf("%s", temp);
        myFree(line, temp);
        exit(0);
    }

    // pipes (1 pipeArr for eache child pipeArr[0] = parent writes to child, [1] = parent reads form child)
    int pipeArr1[2][2];
    int pipeArr2[2][2];
    if(myInitPipes(pipeArr1, pipeArr2) == -1){
        myFree(line, temp);
        myErrorExit("Can't create pipes!");
    }

    // fork
    int c1, c2;
    myFork(&c1, pipeArr1, pipeArr2);
    myFork(&c2, pipeArr2, pipeArr1);

    // closing unused pipe ends
    if  (close(pipeArr1[0][0]) == -1 || close(pipeArr1[1][1]) == -1 || close(pipeArr2[0][0]) == -1 || close(pipeArr2[1][1]) == -1){
        myFree(line, temp);
        myErrorExit("Cant close pipes!");
    }

    // write to children alternating
    write(pipeArr1[0][1], temp, strlen(temp));
    write(pipeArr2[0][1], line, strlen(line));
    int h = 1;
    while(getline(&line, &lSize, stdin) != -1){
        if  (h){
            write(pipeArr1[0][1], line, strlen(line));
            h--;
        }else{
            write(pipeArr1[0][1], line, strlen(line));
            h++;
        }
    }
    myFree(line, temp);

    // closing write ends
    if(close(pipeArr1[0][1]) == -1 || close(pipeArr2[0][1]) == -1){
        myErrorExit("closing pipes");
    }

    // waiting for children
    int cState;
    waitpid(c1, &cState, 0);
    waitpid(c2, &cState, 0);

    // opening read-ends of pipes
    FILE *f1 = fdopen(pipeArr1[1][0] , "r");
    FILE *f2 = fdopen(pipeArr2[1][0] , "r");
    // reading, comparing and printing
    myMergeSort(f1, f2);

    exit(EXIT_SUCCESS);

    return 0;
}




/**
 * Initializes pipes.
 * @brief This function initializes the pipes in the arrays
 * @details The function expects 2 int 2x2 arrays
 * @param arr1 Firtst int 2x2 array
 * @param arr2 Second int 2x2 array
 * @return Returns 0 on sucess, -1 on error
 */
int myInitPipes(int arr1[2][2], int arr2[2][2]){
    for (int i = 0; i < 2; i++)
    {
        if ((pipe(arr1[i]) == -1) || (pipe(arr2[i]) == -1)){
            return -1;
        }        
    }
    return 0;
}

/**
 * Forks the process
 * @brief This function forks the current process
 * @details The function takes care of mapping stdin and stout of child process to pipes,
 * as well as closing the unused parts.
 * @param c Pointer to an int, where the pid of the child will be stored
 * @param usedPipes int 2x2 array containing the two pipes, wich will be remapped
 * @param unusedPipes int 2x2 array containig unused pipes for closing
 */
void myFork(int *c, int usedPipes[2][2], int unusedPipes[2][2]){
    *c = fork();
    if (*c == -1){
        myErrorExit("fork");
    }
    if (*c == 0){
        if(myDup(usedPipes) == -1){
            myErrorExit("dup");
        }
        if (myClosePipeEnds(usedPipes) == -1 || myClosePipeEnds(unusedPipes) == -1){
            myErrorExit("closing pipes");
        }
        if (execl("./forksort", "./forksort", NULL) == -1){
            myErrorExit("execl");
        }
    }
}

/**
 * Closes pipe ends
 * @brief This function closes all pipe ends
 * @details The function closes all pipe ends contained in the 2x2 int array
 * @param arr int 2x2 array containing opend pipe ends
 * @return Returns 0 on sucess, -1 on error
 */
int myClosePipeEnds(int arr[2][2]){
    if (close(arr[0][0]) == -1 || close(arr[0][1]) == -1 ||
        close(arr[1][0]) == -1 || close(arr[1][1]) == -1 ){
        return -1;
    }
    return 0;
}

/**
 * Dupes pipes
 * @brief This function dupes the pipes found in an array
 * @details The function maps [0][0] of an array to stdin and [1][1] to stout, 
 * but does not close anything
 * @param arr int 2x2 array of pipe ends
 * @return Returns 0 on sucess, -1 on error
 */
int myDup(int arr[2][2]){
    dup2(arr[0][0], STDIN_FILENO);
    dup2(arr[1][1], STDOUT_FILENO);
    return 0;    
}

/**
 * Mergesorts lines
 * @brief This function sorts lines alphabetically using mergesort
 * @details The function sorts the lines read from the two files and sorts them
 * using the concept of mergesort
 * @param fd1 First Filedescriptor
 * @param fd2 Second Filedescriptor
 * @return Returns 0 on sucess
 */
int myMergeSort(FILE *fd1, FILE *fd2){
    char *l1 = NULL;
    char *l2 = NULL;
    int h1 = 1;
    int h2 = 1;
    int sw = -1;
    size_t lSize = 0;

    getline(&l1, &lSize, fd1);
    getline(&l2, &lSize, fd2);

    while(h1 && h2){
        sw = myCompPrint(l1, l2);
        if (sw > 0){
            if(getline(&l2, &lSize, fd2) == -1){
                h2--;
            }
        }else{
            if(getline(&l1, &lSize, fd1) == -1){
                h1--;
            }
        }
    }
    while(h1 || h2){
        if(h1){
            printf("%s", l1);
            if(getline(&l1, &lSize, fd1) == -1){
                h1--;
            }
        }else{
            printf("%s", l2);
            if(getline(&l2, &lSize, fd2) == -1){
                h2--;
            }
        }
    }
    myFree(l1, l2);
    fclose(fd1);
    fclose(fd2);

    return 0;
}

/**
 * Compares two strings
 * @brief This function compare two strings
 * @details The function compares two strings usidng strcmp
 * @param l1 Firtst string
 * @param l2 Second string
 * @return Returns 0 if strings are equal, something negative if the first is smaller, something positive if the second is samller
 */
int myCompPrint (char *l1, char *l2){
    int comp = strcmp(l1, l2);
    if(comp > 0){
        printf("%s", l2);
    }else{
        printf("%s", l1);              
    }
    return comp;
}

/**
 * Frees strings
 * @brief This function frees 2 strings
 * @details The function frees 2 previously allocated strings
 * @param a Firtst string
 * @param b Second string
 */
void myFree(char *a, char *b){
    free(a);
    free(b);
}

/**
 * Error Exit
 * @brief This function exits with error message
 * @details The function prints error message to stderr and then 
 * exits with EXIT_FAILURE
 * @param msg string containing the error message
 */
void myErrorExit(char *msg){
    fprintf(stderr, "%s (%d): ERROR: %s\n", PROG_NAME, pid, msg);
    exit(EXIT_FAILURE);
}