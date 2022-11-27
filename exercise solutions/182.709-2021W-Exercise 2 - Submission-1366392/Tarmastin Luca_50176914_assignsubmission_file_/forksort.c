/**@author: Luca Tarmastin, 01633051
*@brief: Recursively sorts a list of Strings from stdin by forking twice and sending half of the list to the child processes 
* by utilizing pipes
*@date: 03.12.2021
**/
#include <stdio.h>
#include "stdlib.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>
#define BUFSIZE 50

char *myprog;
int lines;
char *array[BUFSIZE];

/**
 * @brief Frees allocated array resource at exit
 */
void freeResources(){
    for (int i=0; i<lines; i++) {
        free(array[i]);
    }
}

/** 
 * @brief Wait on Child method
 * @param p Process ID
 * @return Returns Exit status code of Process p
**/
static int wait_for_termination(pid_t p)
{   
    int status = 0;

    if (waitpid(p, &status, 0) == -1) {
        perror("waitpid failed: ");
    }   
    return WEXITSTATUS(status);
}

int main(int argc, char **argv) {
    myprog = argv[0];
    atexit(freeResources);
    lines = 0;
    if(argc > 1){
        exit(EXIT_FAILURE);
    }

    size_t len = 0;
    //char *array[BUFSIZE];
    
    char *inputline = NULL;
    ssize_t nread;
    while ((nread = getline(&inputline, &len, stdin)) != -1) {
        array[lines] = strdup(inputline);
        lines++;
    }

    if(lines == 1){
        printf("%s", array[0]);
        fflush(stdout);
        //free(array[0]);
        exit(EXIT_SUCCESS);
    }

    int pipefdReadFromChild1[2];
    int pipefdWriteToChild1[2];
    int pipefdReadFromChild2[2];
    int pipefdWriteToChild2[2];

    if(pipe(pipefdReadFromChild1)==-1){
        perror("Pipe Read from Child 1: ");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipefdWriteToChild1)==-1){
        perror("Pipe Write to Child 1: ");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipefdReadFromChild2)==-1){
        perror("Pipe Read from Child 2: ");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipefdWriteToChild2)==-1){
        perror("Pipe Write to Child 2: ");
        exit(EXIT_FAILURE);
    }

    pid_t pid_child2;
    pid_t pid_child1 = fork();
    switch (pid_child1) {
        case -1: //Error
            perror("Fork error: ");
            exit(EXIT_FAILURE);
            break;
        case 0: //child
            if(close(pipefdReadFromChild2[0]) == -1 || close(pipefdReadFromChild2[1])==-1){
                perror("Close Pipe Read from Child 2 failed: ");
            }
            if(close(pipefdWriteToChild2[0])==-1 || close(pipefdWriteToChild2[1])==-1){
                perror("Close Pipe Write to Child 2 failed: ");
            }
            if(close(pipefdReadFromChild1[0]) == -1){
                perror("Close Pipe Read from Child 1 [0] failed: ");
            }
            if(close(pipefdWriteToChild1[1]) == -1){
                perror("Close Pipe Write to Child 1 [1] failed: ");
            }
            if(dup2(pipefdWriteToChild1[0], STDIN_FILENO) == -1){
                perror("Failed duplicate of Pipe Write to Child 1: ");
            }
            if(dup2(pipefdReadFromChild1[1], STDOUT_FILENO) == -1){
                perror("Failed duplicate of Pipe Read from Child: ");
            }
            if(close(pipefdReadFromChild1[1])==-1){
                perror("Close Pipe 1[0] in Child 1 failed: ");
            }
            if(close(pipefdWriteToChild1[0])==-1){
                perror("Close Pipe 1[1] in Child 1 failed: ");
            }
            execlp(argv[0], argv[0], NULL);
            perror("Execute error child 1: ");
            exit(EXIT_FAILURE);
            break;
        default: //parent
            switch (pid_child2 = fork()) {
                case -1:
                    perror("error during 2nd fork");
                    exit(EXIT_FAILURE);
                    break;
                case 0: 
                    if(close(pipefdReadFromChild1[0]) == -1 || close(pipefdReadFromChild1[1]) == -1){
                        perror("Close Pipe Read from Child 1 in Child 2 failed: ");
                    }
                    if(close(pipefdWriteToChild1[0]) == -1 || close(pipefdWriteToChild1[1]) == -1){
                        perror("Close Pipe Write to Child 1 in Child 2 failed: ");
                    }
                    if(close(pipefdWriteToChild2[1]) == -1){
                        perror("Close Pipe Write to Child 2 failed: ");
                    }
                    if(close(pipefdReadFromChild2[0]) == -1){
                        perror("Close Pipe Read from Child 2 failed: ");
                    }
                    if(dup2(pipefdWriteToChild2[0], STDIN_FILENO) == -1){
                        perror("Failed duplicate of pipe 2[0]: ");
                    }
                    if(dup2(pipefdReadFromChild2[1], STDOUT_FILENO) == -1){
                        perror("Failed duplicate of pipe 2[1]: ");
                    }
                    if(close(pipefdReadFromChild2[1])==-1){
                        perror("Close Pipe Read from Child 2[1] failed: ");
                    }
                    if(close(pipefdWriteToChild2[0])==-1){
                        perror("Close Pipe Write to Child 2[0] failed: ");
                    }
                    execlp(argv[0], argv[0], NULL);
                    perror("Execute error child 2: ");
                    exit(EXIT_FAILURE);
                    break;
                default:
                    break;
            }
            break;
    }
    if(close(pipefdReadFromChild1[1]) == -1){
        perror("Close Pipe Read from Child 1[1] in Parent failed: ");
    }
    if(close(pipefdWriteToChild1[0]) == -1){
        perror("Close Pipe Write to Child 1[0] in Parent failed: ");
    }
    if(close(pipefdReadFromChild2[1]) == -1){
        perror("Close Pipe Read from Child 2[1] in Parent failed: ");
    }
    if(close(pipefdWriteToChild2[0]) == -1){
        perror("Close Pipe Write to Child 2[0] in Parent failed: ");
    }

    //split array and write to pipes
    int length = lines/2;
    for (size_t i = 0; i < length; i++) {
        dprintf(pipefdWriteToChild1[1], "%s", array[i]);
    }
    if(close(pipefdWriteToChild1[1])==-1){
        perror("Close Pipe 1[1] in Parent failed: ");
    }
    
    for (size_t i = length; i < lines; i++) {
        dprintf(pipefdWriteToChild2[1], "%s", array[i]);
    }
    if(close(pipefdWriteToChild2[1])==-1){
        perror("Close Pipe 2[1] in Parent failed: ");
    }
    
    int stat1 = 0, stat2 = 0;
    stat1 = wait_for_termination(pid_child1);
    stat2 = wait_for_termination(pid_child2);

    if(WEXITSTATUS(stat1) == EXIT_FAILURE || WEXITSTATUS(stat2) == EXIT_FAILURE){
        perror("Child Exit failure: ");
        exit(EXIT_FAILURE);
    }
    
    //read from pipe
    char *sortedarray1[length];
    char *stringchild = NULL;
    len = 0;
    ssize_t nreadc1;
    int position = 0;
    FILE *fin_child1, *fin_child2;
    fin_child1 = fdopen(pipefdReadFromChild1[0], "r");
    fin_child2 = fdopen(pipefdReadFromChild2[0], "r");

    while ((nreadc1 = getline(&stringchild, &len, fin_child1)) != -1) {
        sortedarray1[position] = strdup(stringchild);
        position++;
    }
    fclose(fin_child1);

    char *sortedarray2[lines - length];
    stringchild = NULL;
    ssize_t nreadc2;
    position = 0;
    len = 0;
    while ((nreadc2 = getline(&stringchild, &len, fin_child2)) != -1) {
        sortedarray2[position] = strdup(stringchild);
        position++;
    }
    fclose(fin_child2);
    
    //merge the two ordered arrays and write to stdout
    int c1 = 0;
    int c2 = 0;
    for(int i = 0; i < lines; i++){

        if(c1 == length){
            printf("%s", sortedarray2[c2]);
            c2++;
        }else if(c2 == (lines-length)) {
            printf("%s", sortedarray1[c1]);
            c1++;
        }else if(strcmp(sortedarray1[c1], sortedarray2[c2]) < 0){
            printf("%s", sortedarray1[c1]);
            c1++;
        }else { 
            printf("%s", sortedarray2[c2]);
            c2++;
        }
    }

    for (int i=0; i<(lines-length); i++) {
        if(i < length) {
            free(sortedarray1[i]);
        }
        free(sortedarray2[i]);
    }
    exit(EXIT_SUCCESS);
}