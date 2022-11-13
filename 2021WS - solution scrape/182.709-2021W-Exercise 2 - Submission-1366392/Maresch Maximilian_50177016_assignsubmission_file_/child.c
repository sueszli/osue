/**
 * @author Maximilian Maresch
 */ 

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "child.h"

pid_t setupChild(int stdinPipe[], int stdoutPipe[], char* line1, char* line2, char* argv[]) {
    pid_t pid = fork();

    switch (pid) {
        case -1:
            fprintf(stderr, "[%s] Cannot fork for: %s %s\n", argv[0], line1, line2);
            break;
        case 0:
            close(stdinPipe[1]);
            dup2(stdinPipe[0], STDIN_FILENO);
            close(stdinPipe[0]);

            close(stdoutPipe[0]);
            dup2(stdoutPipe[1], STDOUT_FILENO);
            close(stdoutPipe[1]);

            execlp("./intmul", argv[0], (char *) NULL);
            break;
        default:
            close(stdinPipe[0]);

            FILE *out;
    
            if ((out = fdopen(stdinPipe[1], "w")) == NULL) {
                fprintf(stderr, "[%s] fdopen failed\n", argv[0]);
                return -1;
            }

            int puts1 = fputs(line1, out);
            int puts2 = fputs("\n", out);
            int puts3 = fputs(line2, out);
            int puts4 = fputs("\n", out);

            if (puts1 == EOF || puts2 == EOF || puts3 == EOF || puts4 == EOF) {
                fprintf(stderr, "[%s] fputs failed\n", argv[0]);
                return -1;
            }

            fflush(out);

            fclose(out);

            close(stdinPipe[1]);
            close(stdoutPipe[1]);

            break;
    }

    return pid;
}

char* readResultFromChild(int stdoutPipe[], char* argv[]) {
    FILE *childIn;
    if ((childIn = fdopen(stdoutPipe[0], "r")) == NULL) {
        fprintf(stderr, "[%s] fdopen failed\n", argv[0]);
        return NULL;
    }
    
    char* childResult = NULL;
    size_t childLen = 0;
    ssize_t childLineSize = 0;
    if ((childLineSize = getline(&childResult, &childLen, childIn)) == -1) {
        fprintf(stderr, "[%s] getline failed\n", argv[0]);
        return NULL;
    }

    return childResult;
}