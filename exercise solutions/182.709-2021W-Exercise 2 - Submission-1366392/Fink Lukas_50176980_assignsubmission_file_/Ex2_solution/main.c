/**
 * @file main.c
 * @author Lukas Fink 11911069
 * @date 03.12.2021
 *
 * @brief Main program module.
 * 
 * @details This program solves the closest neighbor problem with a divide and conquer approach
 * using fork, pipe and exev to solve it in parallel
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include "datatypes.h"

/**
 * usage function
 * @brief function to print a usage message and exits
 * @details exits with EXIT_FAILURE
 * @param message message to be printed
 * @param programName name of the program
 **/
void usage (char *message, char *programName) {
    fprintf(stderr, "Usage: ./cpair < 1.txt\n1.txt: 1.0 1.0[<newline>2.0 3.0]*\n%s: failure: %s\n", programName, message);
    exit(EXIT_FAILURE);
}

/**
 * exit function
 * @brief function to print a message and exits
 * @details exits with EXIT_FAILURE
 * @param message message to be printed
 * @param programName name of the program
 **/
void exit_error(char *message, char *programName) {
    fprintf(stderr, "%s: %s\n%s\n", programName, message, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * outOfMemory function
 * @brief function to print that the program is out of memory and exits
 * @details exits with EXIT_FAILURE
 * @param programName name of the program
 **/
void outOfMemory (char *programName) {
    fprintf(stderr, "%s: out of memory!\n", programName);
    exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief The program starts here. It should be called like './cpair < 1.txt'.
 * 1.txt should be like: 1.0 1.0[<newline>2.0 3.0]*
 * @details It reads from stdin points and recursivly solves the closest neighbor problem
 * @param argc The argument counter. Must be 1
 * @param argv The argument vector. Must contain only the program name in argv[0]
 * @return Returns EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 **/
int main (int argc, char *argv[]) {
    char* programName = argv[0];
    if (argc != 1) {
        usage("wrong input format! No arguments allowed!", programName);
        exit(EXIT_FAILURE);
    }

    int size = 8;
    struct coord *input = malloc(size * sizeof(struct coord));
    if (input == NULL) {
        outOfMemory(programName);
    }

    char *buffer = NULL;
    size_t len = 0;
    ssize_t read;
    char rest = '\0';
    int count = 0; // number of entries in the coord array
    while ((read = getline(&buffer, &len, stdin)) != -1) {
        if (count > size -1) {
            size += 64;
            input = realloc(input, size * sizeof(struct coord));
            if (input == NULL) {
                free(input);
                free(buffer);
                outOfMemory(programName);
            }
        }
        if (buffer[0] == '\n') {
            free(input);
            free(buffer);
            usage("wrong input format! no points in a line!", programName);
        }
        char *restX = NULL;
        input[count].x = strtof(buffer, &restX);
        if (restX == NULL || restX[0] == '\n' || restX[0] == '\0') {        // only one float
            free(input);
            free(buffer);
            usage("wrong input format! only one float", programName);
        }
        char *restY = NULL;
        input[count].y = strtof(restX, &restY);
        rest = restY[0];
        if (rest != '\n' && rest != '\0') {     // too much input
            free(input);
            free(buffer);
            usage("wrong input format!", programName);
        }

        count++;
    }
    free(buffer);

    if (count == 0) {
        free(input);
        usage("no points!", programName);
    }

    if (count == 1) {
        free(input);
        exit(EXIT_SUCCESS);
    }

    if (count == 2) {
        printCoordArray(stdout, input, count);
        free(input);
        exit(EXIT_SUCCESS);
    }

    // count > 2
    // divide
    int inputSize = count;
    double mean = arithmeticMeanX(input, inputSize);
    int Nless = countLEG(input, inputSize, mean, 'l');
    int Nequal = countLEG(input, inputSize, mean, 'e');
    int Ngreater = countLEG(input, inputSize, mean, 'g');
    if (Nless == -1 || Nequal == -1 || Ngreater == -1) {
        free(input);
        exit_error("countLEG failed", programName);
    }

    int sizeLEq = Nless + (Nequal + 1)/2;
    int sizeGr = Ngreater + Nequal/2;
    struct coord lessEq[sizeLEq];
    struct coord greater[sizeGr];
    if (divide(lessEq, greater, input, inputSize, mean) != 0) {
        free(input);
        exit_error("divide failed", programName);
    }
    free(input);
/*  just for testing   
    fprintf(stderr, "lessEq:\n");
    printCoordArray(stderr, lessEq, sizeLEq);
    fprintf(stderr, "greater:\n");
    printCoordArray(stderr, greater, sizeGr);
*/
    

// make child processes
// left child
    int pipefd_wr_left[2];
    pipe(pipefd_wr_left);   // create pipe to write to the child
    int pipefd_rd_left[2];
    pipe(pipefd_rd_left);   // create pipe to read from the child

    pid_t pid_left = fork();
    switch (pid_left) {
        case -1:
            exit_error("error when calling fork()!", programName);
            break;
        case 0:
            // child task
            // pipe to read from
            close(pipefd_wr_left[1]);  // close unused write end
            dup2(pipefd_wr_left[0], STDIN_FILENO);  // stdin is now directed to the read end of the pipe
            close(pipefd_wr_left[0]);               // close read end of the pipe
            // pipe to write to
            close(pipefd_rd_left[0]);   // close unused read end
            dup2(pipefd_rd_left[1], STDOUT_FILENO); // stdout is now directed to the write end of the pipe
            close(pipefd_rd_left[1]);               // close write end of the pipe
            // start the program from the beginning
            execlp("./cpair", "./cpair", NULL);
            // if program is here, an error occured
            exit_error("Cannot execlp!", programName);
            break;
        default:
            // parent task
            // pipe to write to
            close(pipefd_wr_left[0]);   // close unused read end
            // pipe to read from
            close(pipefd_rd_left[1]);   // close unused write end
            break;
    }
// right child
    int pipefd_wr_right[2];
    pipe(pipefd_wr_right);   // create pipe to write to the child
    int pipefd_rd_right[2];
    pipe(pipefd_rd_right);   // create pipe to read from the child

    pid_t pid_right = fork();
    switch (pid_right) {
        case -1:
            exit_error("error when calling fork()!", programName);
            break;
        case 0:
            // child task
            // pipe to read from
            close(pipefd_wr_right[1]);  // close unused write end
            dup2(pipefd_wr_right[0], STDIN_FILENO);  // stdin is now directed to the read end of the pipe
            close(pipefd_wr_right[0]);               // close read end of the pipe
            // pipe to write to
            close(pipefd_rd_right[0]);   // close unused read end
            dup2(pipefd_rd_right[1], STDOUT_FILENO); // stdout is now directed to the write end of the pipe
            close(pipefd_rd_right[1]);               // close write end of the pipe
            // start the program from the beginning
            execlp("./cpair", "./cpair", NULL);
            // if program is here, an error occured
            exit_error("Cannot execlp!", programName);
            break;
        default:
            // parent task
            // pipe to write to
            close(pipefd_wr_right[0]);   // close unused read end
            // pipe to read from
            close(pipefd_rd_right[1]);   // close unused write end
            break;
    }    

// write to the childs
    // write to left child
    FILE* write_left = fdopen(pipefd_wr_left[1], "w");
    if (write_left == NULL) {
        fclose(write_left);
        exit_error("fdopen failed!", programName);
    }
    printCoordArray(write_left, lessEq, sizeLEq);
    if (fclose(write_left) == EOF) {
        exit_error("fclose failed!", programName);
    }
    // write to right child
    FILE* write_right = fdopen(pipefd_wr_right[1], "w");
    if (write_right == NULL) {
        fclose(write_right);
        exit_error("fdopen failed!", programName);
    }
    printCoordArray(write_right, greater, sizeGr);
    if (fclose(write_right) == EOF) {
        exit_error("fclose failed!", programName);
    }

// read from the childs
    // read from left child
    struct coord closestLeft[2];
    FILE *read_left = fdopen(pipefd_rd_left[0], "r");
    if (read_left == NULL) {
        fclose(read_left);
        exit_error("fdopen failed!", programName);
    }
    int rFC_ret_l = readFromChild(closestLeft, read_left);
    if (rFC_ret_l == -1) {
        fprintf(stderr, "readFromChild failed!\n");
        exit(EXIT_FAILURE);
    }
    if (rFC_ret_l == +1) {
        // child had only one point and printed nothing to the parent
    }
    if (fclose(read_left) == EOF) {
        exit_error("fclose failed!", programName);
    }
    // read from right child
    struct coord closestRight[2];
    FILE *read_right = fdopen(pipefd_rd_right[0], "r");
    if (read_right == NULL) {
        fclose(read_right);
        exit_error("fdopen failed!", programName);
    }
    int rFC_ret_r = readFromChild(closestRight, read_right);
    if (rFC_ret_r == -1) {
        fprintf(stderr, "%s: readFromChild failed!\n", programName);
        exit(EXIT_FAILURE);
    }
    if (rFC_ret_r == +1) {
        // child had only one point and printed nothing to the parent
    }
    if (fclose(read_right) == EOF) {
        exit_error("fclose failed!", programName);
    }

// shortest distance between left set and right set
    struct coord closestBetween[2];
    int sDB_ret = shortestDistanceBetween(closestBetween, lessEq, sizeLEq, greater, sizeGr);
    if (sDB_ret == -1) {
        fprintf(stderr, "%s: shortestDistanceBetween failed!\n", programName);
        exit(EXIT_FAILURE);
    }
    // compare the three distances and write the closest pair to stdout
    int wCTS_ret = writeClosestToStdout(closestLeft, rFC_ret_l, closestRight, rFC_ret_r, closestBetween, sDB_ret);
    if (wCTS_ret == -1) {
        fprintf(stderr, "%s: writeClosestToStdout failed! lef + right + between has only one point!\n", programName);
        exit(EXIT_FAILURE);
    }

    /* just for testing 
    if (rFC_ret_r == +1) {
        printCoordArray(stdout, closestLeft, 2);
    } else {
        printCoordArray(stdout, closestRight, 2);
    }
    */

// write closest pair to stdout

// terminate the two childs that were spawned
    int status;
    pid_t child_pid;
    int n;
    for (n = 0; n < 2; n++) {
        child_pid = wait(&status);
        if (child_pid == -1) {
            if (errno != EINTR) {
                exit_error("Cannot wait!", programName);
            }
        }
        if (WEXITSTATUS(status) != EXIT_SUCCESS) {
            fprintf(stderr, "%s: A child did not terminate properly!\n", programName);
            exit(EXIT_FAILURE);
        }
    }

// free recources not neccercary at this point because everything has been freed up to now

    exit(EXIT_SUCCESS);
}