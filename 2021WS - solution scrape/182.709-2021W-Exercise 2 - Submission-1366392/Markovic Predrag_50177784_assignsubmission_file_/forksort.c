/***
 * @author: Markovic Predrag
 * @brief: Main module
 * @details: forksort.c is the main module and its purpose is to start two child processes
 *           which use mergesort to sort the input provided by stdin.
 * @date: 8.12.2021
 ***/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include "helper_functions.h"

/***
 * @brief: Tries to convert STR to number type
 * @details: This procedure takes a string and pointer of the variable and tries to convert the string in a number type
 *           and save it to the num variable. On error occurence its stops the whole program.
 * @param: STR - String to convert to number type
 * @param: num - Pointer to a variable in which the result is saved
 * @return: void
 ***/
static void try_convert_string_to_number(const char* STR, long int *num) {
    char* endptr;
    *num = strtol(STR, &endptr, 10);

    if((errno == ERANGE && (*num == LONG_MAX || *num == LONG_MIN))
                || (errno != 0 && *num == 0)) {
        fprintf(stderr, "./forksort: Could not convert argument to number!\n");
        exit(EXIT_FAILURE);
    }

    if(endptr == STR) {
        fprintf(stderr, "./forksort: Could not convert argument to number - No numbers found!\n");
        exit(EXIT_FAILURE);
    }

}


/***
 * @brief: Determine the lines in stdin
 * @details: Tries to read the line by calling wc via pipe and returns the line count
 * @return: long int
 ***/
static long int read_stdin_rows() {
    int fds1[2];
    int fds2[2];

    int pres = pipe(fds1);
    checkPipeRes(pres);

    pres = pipe(fds2);
    checkPipeRes(pres);

    // ERROR HANDLING

    int pid = fork();
    int status;
    int cres;
    long int rows;


    if(pid == 0) {

        cres = close(fds2[0]);
        checkCloseRes(cres);

        cres = close(fds1[1]);
        checkCloseRes(cres);

        cres = close(STDOUT_FILENO);
        checkCloseRes(cres);

        cres = close(STDIN_FILENO);
        checkCloseRes(cres);

        int dres = dup2(fds2[1], STDOUT_FILENO);
        checkDubRes(dres);

        cres = close(fds2[1]);
        checkCloseRes(cres);

        dres = dup2(fds1[0], STDIN_FILENO);
        checkDubRes(dres);

        cres = close(fds1[0]);
        checkCloseRes(cres);

        execlp("wc", "wc" , "-l", NULL);

        fprintf(stderr, "./forksort: Process could not terminate\n");
        exit(EXIT_FAILURE);
    }

    else if( pid == -1) {
        fprintf(stderr, "./forksort: Cannot fork!\n");
        exit(EXIT_FAILURE);
    }

    else {
        cres = close(fds1[0]);
        checkCloseRes(cres);

        cres = close(fds2[1]);
        checkCloseRes(cres);

        char buf[1024];
        memset(buf, 0, sizeof(buf));


        FILE* writ = fdopen(fds1[1], "w");

        if(writ == NULL) {
            fprintf(stderr,"./forksort: Could not open file stream to write: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }


        while(fgets(buf, sizeof(buf), stdin) != NULL) {
            fputs(buf, writ);
        }


        int ffres = fflush(writ);

        if(ffres < 0) {
            fprintf(stderr,"./forksort: Flush error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }


        cres = close(fds1[1]);
        checkCloseRes(cres);
        fclose(writ);

        // Wait for child process
        pid_t result = wait(&status);


        if(result != -1 && WEXITSTATUS(status) == EXIT_SUCCESS) {
            char number[BUFSIZ];
            memset(number, 0, sizeof(number));

            FILE* readSource = fdopen(fds2[0], "r");
            if(readSource == NULL) {
                fprintf(stderr,"./forksort: Could not open stream to read: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            fgets(number, sizeof(number), readSource);
            cres = close(fds2[0]);
            checkCloseRes(cres);

            try_convert_string_to_number(number, &rows);

        } else {
            fprintf(stderr, "./forksort: Program EXIT FAILURE\n");
            exit(EXIT_FAILURE);
        }

    }

    return rows;

}

int main(int argc, char* argv[]) {

        long int rows = 0;

        if(argv[1] != NULL && strcmp(argv[1], "child") == 0)
          try_convert_string_to_number(argv[2], &rows);

        else
          rows = read_stdin_rows();

        int status;
        rewind(stdin);

        if(rows == 0) {
            exit(EXIT_FAILURE);
          }


        else if (rows == 1) {
            char buf[1024];
            memset(buf, 0, sizeof(buf));

            while(fgets(buf, sizeof(buf), stdin) != NULL) {
                printf("%s", buf);
            }

            exit(EXIT_SUCCESS);
        }

        else {
            // Create two child processes
            long int splitIndex = rows / 2;

            int pipefdchild11[2];
            int pipefdchild12[2];

            int pipefdchild21[2];
            int pipefdchild22[2];

            pid_t child1;
            int cres;
            int pres;
            int dres;

            pres = pipe(pipefdchild11);
            checkPipeRes(pres);

            pres = pipe(pipefdchild12);
            checkPipeRes(pres);

            child1 = fork();

            if(child1 == 0) {
                // CHILD 1

                cres = close(STDOUT_FILENO);
                checkCloseRes(cres);

                cres = close(STDIN_FILENO);
                checkCloseRes(cres);

                cres = close(pipefdchild11[1]);
                checkCloseRes(cres);

                cres = close(pipefdchild12[0]);
                checkCloseRes(cres);

                dres = dup2(pipefdchild11[0], STDIN_FILENO);
                checkDubRes(dres);

                cres = close(pipefdchild11[0]);
                checkCloseRes(cres);

                dres = dup2(pipefdchild12[1], STDOUT_FILENO);
                checkDubRes(dres);

                cres = close(pipefdchild12[1]);
                checkCloseRes(cres);

                char buf[2048];
                memset(buf, 0, sizeof(buf));
                snprintf(buf, sizeof(buf), "%ld", splitIndex);

                execlp("./forksort", "./forksort","child", buf, NULL);

                fprintf(stderr, "./forksort: Child process could not terminate!\n");
                exit(EXIT_FAILURE);
            }

            else if(child1 < 0) {
                fprintf(stderr, "./forksort: Cannot fork to split stdin: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }

            else {

                // PARENT
                cres = close(pipefdchild11[0]);
                checkCloseRes(cres);

                cres = close(pipefdchild12[1]);
                checkCloseRes(cres);

                char buf[1024];
                memset(buf, 0, sizeof(buf));

                FILE* writeStream = fdopen(pipefdchild11[1], "w");


                if(writeStream == NULL) {
                    fprintf(stderr, "./forksort: Could not open stream to write: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                int currentLine;

                for(currentLine = 0; currentLine < splitIndex; currentLine++) {
                    fgets(buf,sizeof(buf), stdin);
                    fputs(buf, writeStream);
                }

                int fres = fflush(writeStream);

                if(fres < 0) {
                    fprintf(stderr, "./forksort: Could not flush writeStream: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                fclose(writeStream);


                pipe(pipefdchild21);
                pipe(pipefdchild22);

                pid_t child2 = fork();

                if(child2 == 0) {
                    // CHILD 1

                    cres = close(STDOUT_FILENO);
                    checkCloseRes(cres);

                    cres = close(STDIN_FILENO);
                    checkCloseRes(cres);

                    cres = close(pipefdchild21[1]);
                    checkCloseRes(cres);

                    cres = close(pipefdchild22[0]);
                    checkCloseRes(cres);

                    dres = dup2(pipefdchild21[0], STDIN_FILENO);
                    checkDubRes(dres);

                    cres = close(pipefdchild21[0]);
                    checkCloseRes(cres);

                    dres = dup2(pipefdchild22[1], STDOUT_FILENO);
                    checkDubRes(dres);

                    cres = close(pipefdchild22[1]);
                    checkCloseRes(cres);

                    char buf[2048];
                    memset(buf, 0, sizeof(buf));
                    snprintf(buf, sizeof(buf), "%ld", rows - splitIndex);

                    execlp("./forksort", "./forksort","child", buf, NULL);

                    fprintf(stderr, "./forksort: Child process could not terminate!\n");
                    exit(EXIT_FAILURE);
                }

                else if(child2 < 0) {
                    fprintf(stderr, "./forksort: Cannot fork to split stdin: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                else {

                    // PARENT
                    cres = close(pipefdchild21[0]);
                    checkCloseRes(cres);

                    cres = close(pipefdchild22[1]);
                    checkCloseRes(cres);

                    char buf[1024];
                    memset(buf, 0, sizeof(buf));

                    writeStream = fdopen(pipefdchild21[1], "w");

                    for(; currentLine < rows; currentLine++) {
                        fgets(buf,sizeof(buf), stdin);
                        fputs(buf, writeStream);
                      }

                    fclose(writeStream);

                  }
              }

              int child_counter = 0;
              do {
                wait(&status);

                if(WEXITSTATUS(status) == EXIT_SUCCESS) {
                    child_counter++;
                }

                else if(WEXITSTATUS(status) == EXIT_FAILURE) {
                   fprintf(stderr, "./forksort: Child EXIT_FAILURE\n");
                    exit(EXIT_FAILURE);
                }

                else if(__WCOREDUMP(status)) {
                    fprintf(stderr, "./forksort: Child SEGFAULT\n");
                    exit(EXIT_FAILURE);
                }
                else {
                    fprintf(stderr, "./forksort: Child OTHER FAILURE\n");
                    exit(EXIT_FAILURE);
                }
            }
           while(child_counter != 2);


          // MERGE ALGORITHM
           writeStreamsToStdout(pipefdchild12[0], pipefdchild22[0]);

        }


    return EXIT_SUCCESS;
}
