/**
 * @name forksort
 * @author Christian Perzy [11809921]
 *
 * @brief Assignment 2 for OSUE21
 *
 * This program sorts a list from stdin
 * alphabetically
 **/

#include "forksort.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

char * progname; /**< program name */

/**
 * Exit with error message.
 * @brief This Functions prints an error message (with program name
 * and errno string) and terminate the program with EXIT_FAILURE
 * @details global variables: progname
 * @param message Additional Message
 */
static void error_exit(const char *message) {
    fprintf(stderr, "[%s] %s: %s\n", progname, message, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * Setup Child
 * @brief Close unused pipes, redirect stdin and stdout and load original
 * program image
 * @details global variables: progname
 * @param pipe_in Pipe where the child get data
 * @param pipe_out Pipe where the child submit data
 * @param other_pipe1 Pipe for the other child (opened in parent)
 * @param other_pipe2 Pipe for the other child (opened in parent)
 */
static void setup_child(int *pipe_in, int *pipe_out, int *other_pipe1, int *other_pipe2) {
    int arr_size = 8;
    int closed[arr_size];

    // close unused pipes
    closed[0] = close(pipe_in[1]);
    closed[1] = close(pipe_out[0]);

    closed[2] = close(other_pipe1[0]);
    closed[3] = close(other_pipe1[1]);
    closed[4] = close(other_pipe2[0]);
    closed[5] = close(other_pipe2[1]);

    // redirect stdin and stdout
    int err[2];
    err[0] = fclose(stdin);
    err[1] = fclose(stdout);
    if (err[0] == EOF || err[1] == EOF) error_exit("Close stdin/stdout failed!");

    err[0] = dup2(pipe_in[0],STDIN_FILENO);
    err[1] = dup2(pipe_out[1],STDOUT_FILENO);
    if (err[0] == -1 || err[1] == -1) error_exit("Redirect stdin/stdout failed!");

    closed[6] = close(pipe_in[0]);
    closed[7] = close(pipe_out[1]);

    for (int i = 0; i < arr_size; i++) {
        if (closed[i] == -1) {
            char ms[22];
            sprintf(ms,"%s%d%s","close pipe failed (",i,")");
            error_exit(ms);
        }
    }

    // load program image
    execl("forksort", progname, NULL);
    error_exit("Loading Program Image failed!");
}

/**
 * Main function of the Program
 * @brief This function contains the main functionality for the program
 * @details global variables: progname quit
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS resp. EXIT_FAILURE if an error occurs.
 */
int main(int argc, char ** argv) {
    progname = argv[0];

    char *fst = NULL;
    char *snd = NULL;
    char *line = NULL;
    size_t len = 0;

    // read the first two lines
    if(getline(&line,&len,stdin) != -1) {
        fst = malloc(len);
        if (fst == NULL) error_exit("malloc failed");
        strcpy(fst,line);
    }
    if(getline(&line,&len,stdin) != -1) {
        snd = malloc(len);
        if (snd == NULL) error_exit("malloc failed");
        strcpy(snd,line);
    }

    free(line);
    
    if (fst == NULL) { // return if no is given
        exit(EXIT_SUCCESS);
    } else if (snd == NULL) { // if one line is given; print line and return
        fprintf(stdout,"%s",fst);
        free(fst);
    } else {
        int pipeC1[2], pipeC2[2], pipeS1[2], pipeS2[2];

        if (pipe(pipeC1) == -1) error_exit("Open Pipe C1 failed");
        if (pipe(pipeC2) == -1) error_exit("Open Pipe C2 failed");
        if (pipe(pipeS1) == -1) error_exit("Open Pipe S1 failed");
        if (pipe(pipeS2) == -1) error_exit("Open Pipe S2 failed");

        pid_t pidC1, pidC2;

        // Generate first Child
        pidC1 = fork();
        if (pidC1 == -1) {
            error_exit("fork C1 failed");
        } else if (pidC1 == 0) {
            setup_child(pipeC1,pipeS1,pipeC2,pipeS2);
        }

        // Generate second Child
        pidC2 = fork();
        if (pidC2 == -1) {
            error_exit("fork C2 failed");
        } else if (pidC2 == 0) {
            setup_child(pipeC2,pipeS2,pipeC1,pipeS1);
        }

        int cl_error_size = 8;
        int cl_err[cl_error_size];

        // close unused pipeends
        cl_err[0] = close(pipeC1[0]);
        cl_err[1] = close(pipeS1[1]);
        cl_err[2] = close(pipeC2[0]);
        cl_err[3] = close(pipeS2[1]);

        // send data to the childs
        FILE * out1 = fdopen(pipeC1[1],"w");
        FILE * out2 = fdopen(pipeC2[1],"w");
        if (out1 == NULL || out2 == NULL) error_exit("fdopen pipe failed");

        int fputerr[2];
        fputerr[0] = fputs(fst,out1);
        fputerr[1] = fputs(snd,out2);
        if (fputerr[0] == EOF || fputerr[1] == EOF) error_exit("fputs failed");

        free(fst);
        free(snd);
        line = NULL;

        int d = 1;
        while (getline(&line,&len,stdin) != -1) {
            if ((d % 2) == 1) {
                fputerr[0] = fputs(line,out1);
            } else {
                fputerr[0] = fputs(line,out2);
            }
            if (fputerr[0] == EOF) error_exit("fputs failed");
            d++;
        }

        cl_err[4] = fclose(out1);
        cl_err[5] = fclose(out2);

        free(line);

        // read data from the childs
        char * str1 = NULL;
        char * str2 = NULL;
        FILE * solution1 = fdopen(pipeS1[0],"r");
        FILE * solution2 = fdopen(pipeS2[0],"r");
        if (solution1 == NULL || solution2 == NULL) error_exit("fdopen pipe failed");
        int loaded1 = 0;
        int loaded2 = 0;
        int read1 = 0;
        int read2 = 0;

        while (1) {
            if (loaded1 == 0) {
                read1 = getline(&str1,&len,solution1);
                loaded1 = (read1 > 0) ? 1 : 0;
            }

            if (loaded2 == 0) {
                read2 = getline(&str2,&len,solution2);
                loaded2 = (read2 > 0) ? 1 : 0;
            }

            if ( read1 == -1 && read2 == -1 ) break;

            if (loaded1 == 1 && loaded2 == 1) {
                if (strcmp(str1,str2) < 0) {
                    fputerr[0] = fputs(str1,stdout);
                    loaded1 = 0;
                } else {
                    fputerr[0] = fputs(str2,stdout);
                    loaded2 = 0;
                }
            } else if (loaded1 == 1 && read2 == -1) {
                fputerr[0] = fputs(str1,stdout);
                loaded1 = 0;
            } else if (loaded2 == 1 && read1 == -1) {
                fputerr[0] = fputs(str2,stdout);
                loaded2 = 0;
            }
            if (fputerr[0] == EOF) error_exit("fputs failed");
        }

        cl_err[6] = fclose(solution1);
        cl_err[7] = fclose(solution2);

        for (int i = 0; i < cl_error_size; i++) {
            if (cl_err[i] != 0) {
                char ms[30];
                sprintf(ms,"%s%d%s","close/fclose pipe failed (",i,")");
                error_exit(ms);
            }
        }

        free(str1);
        free(str2);

        int status1, status2;
        waitpid(pidC1,&status1,0);
        waitpid(pidC2,&status2,0);

        if (WEXITSTATUS(status1) != EXIT_SUCCESS || WEXITSTATUS(status2) != EXIT_SUCCESS) {
            error_exit("Child hasn't terminated successful");
        }
    }

    exit(EXIT_SUCCESS);
}