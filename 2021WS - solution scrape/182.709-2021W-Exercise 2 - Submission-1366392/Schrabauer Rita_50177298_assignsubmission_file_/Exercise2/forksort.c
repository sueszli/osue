/**
 * @file forksort.c
 * @author Rita Schrabauer (e12025342@student.tuwien.ac.at)
 * @brief Implementation of the Mergesort-Algorithm by using the function fork()
 * @version 0.1
 * @date 2021-12-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */
// Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * @brief This struct contains 3 variables
 * id ... will contain the process id
 * pipe_in ... is the array for the input-pipe
 * pipe_out ... is the array for the output-pipe
 * 
 */
typedef struct child {
    int id;
    int pipe_in[2];
    int pipe_out[2];
} CHILD;

// Prototypes
/**
 * @brief Usage function, exits program with EXIT_FAILURE
 * 
 * @param prog is the name of the program
 */
void usage(char* prog);
/**
 * @brief initializes the child pointed to by c
 * 
 * @param c is the pointer to the child to be initialized
 */
void initChild(CHILD* c);
/**
 * @brief Implementation of Mergesort (4 simple steps):
 * 1.) The first of the remaining elements which are read from the left pipe and the right pipe are being compared.
 * 2.) The smaller elemnt is written to stdout.
 * 3.) Repeat steps 1. and 2.
 * 4.) If there are no elements left to read from either one of the pipes, the remaining elements of the other pipe are written to stdout.
 * 
 * @param left is the pointer to the left child
 * @param right is the pointer to the right child
 */
void sort(CHILD* left, CHILD* right);

// Implementation
/**
 * @brief Main function
 * If there are any arguments passed, the usage function is called.
 * Otherwise:
 * If there is only one line passed on in stdin, the line is written to stdout right away. Then the program is terminated with EXIT_SUCCESS.
 * Otherwise:
 * Both children are initalized by calling the function initChild.
 * The lines are alternately written to the left and the right child.
 * Then the program waits for the children to terminate.
 * When the children have terminated, the function sort is called.
 * Afterwards the program is terminated with EXIT_SUCCESS.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 * If error occurs, EXIT_FAILURE is returned, else EXIT_SUCCESS is returned
 */
int main(int argc, char** argv){
    if(argc > 1){
        usage(argv[0]);
    }

    // Initializing children
    CHILD leftChild;
    CHILD rightChild;

    // Reading lines
    char* line1, *line2;
    size_t len = 0;
    ssize_t ret1;
    char halfLines = 0;

    FILE* lfile;
    FILE* rfile;

    while(((ret1 = getline(&line1, &len, stdin)) != -1) && (getline(&line2, &len, stdin) != -1)){
        if(halfLines == 0){
            initChild(&leftChild);
            initChild(&rightChild);

            lfile = fdopen(leftChild.pipe_in[1], "w");
            if(lfile == NULL){
                fprintf(stderr, "Reading data: Could not open left pipe as child");
                exit(EXIT_FAILURE);
            }
            rfile = fdopen(rightChild.pipe_in[1], "w");
            if(rfile == NULL){
                fprintf(stderr, "Reading data: Could not open right pipe as file");
                exit(EXIT_FAILURE);
           }
        }
        halfLines++;
        fprintf(lfile, "%s", line1);
        fprintf(rfile, "%s", line2);
    }
    if(ret1 != -1) {
        if(halfLines == 0) {
            fprintf(stdout, "%s", line1);
            return EXIT_SUCCESS;
        } else{
            fprintf(lfile, "%s", line1);
            fclose(lfile);
            fclose(rfile);
            close(leftChild.pipe_in[1]);
            close(rightChild.pipe_in[1]);
        }
    } else{
        if(halfLines == 0){
            fprintf(stderr, "No input!");
        } else {
            fclose(lfile);
            fclose(rfile);
            close(leftChild.pipe_in[1]);
            close(rightChild.pipe_in[1]);
        }
    }
    free(line1);
    free(line2);
    
    // Wait
    int status;
    waitpid(leftChild.id, &status, 0);
    waitpid(rightChild.id, &status, 0);

    // Mergesort durchführen
    sort(&leftChild, &rightChild);
    return EXIT_SUCCESS;
}

/**
 * @brief Implementation of usage
 * 
 * @param prog 
 */
void usage(char* prog) {
    fprintf(stderr, "Usage: %s\n", prog);
    exit(EXIT_FAILURE);
}

/**
 * @brief Implementation of initChild
 * 
 * @param c 
 */
void initChild(CHILD *c) {
    // Initialize Pipes
    if(pipe(c->pipe_in) == -1) {
        fprintf(stderr, "pipe_in failed");
        exit(EXIT_FAILURE);
    }

    if(pipe(c->pipe_out) == -1){
        fprintf(stderr, "pipe_out failed");
        exit(EXIT_FAILURE);
    }

    // Fork and close Pipe-ends
    c->id = fork();
    switch (c->id)
    {
    case -1:
        fprintf(stderr, "Fork failed");
        exit(EXIT_FAILURE);
        break;
    case 0:
        //close Pipe-ends
        close(c->pipe_in[1]);
        close(c->pipe_out[0]);

        //redirect stdin and stdout
        if(dup2(c->pipe_in[0], STDIN_FILENO) == -1){
            fprintf(stderr, "pipe_in: dup failed");
            exit(EXIT_FAILURE);
        }
        if(dup2(c->pipe_out[1], STDOUT_FILENO) == -1){
            fprintf(stderr, "pipe_out: dup failed");
        }

        //execute program
        if(execlp("./forksort", "forksort", NULL) == -1){
            fprintf(stderr, "execlp failed");
        }
    default:
        close(c->pipe_in[0]);
        close(c->pipe_out[1]);
        break;
    }
}

/**
 * @brief Implementation of sort
 * 
 * @param left 
 * @param right 
 */
void sort(CHILD* left, CHILD* right){
    FILE* lfile = fdopen(left->pipe_out[0], "r");
    if(lfile == NULL){
        fprintf(stderr, "Could not open left pipe as child");
        exit(EXIT_FAILURE);
    }
    FILE* rfile = fdopen(right->pipe_out[0], "r");
    if(rfile == NULL){
        fprintf(stderr, "Could not open right pipe as file");
        exit(EXIT_FAILURE);
    }

    char *rline;
    size_t len = 0;
    ssize_t retr;

    retr = getline(&rline, &len, rfile);

    char *lline;

    while (getline(&lline, &len, lfile) != -1)
    {
        while((retr != -1) && (strcmp(lline, rline) >= 0)){
            fprintf(stdout, "%s", rline);
            retr = getline(&rline, &len, rfile);
        }
        fprintf(stdout, "%s", lline);
    }

    while (retr != -1)
    {
        fprintf(stdout, "%s", rline);
        retr = getline(&rline, &len, rfile);
    }
    
    fclose(lfile);
    fclose(rfile);
}