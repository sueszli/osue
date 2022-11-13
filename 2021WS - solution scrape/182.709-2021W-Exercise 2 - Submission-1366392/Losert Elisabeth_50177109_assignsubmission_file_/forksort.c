/**
 * @author 11908100 Elisabeth Losert
 * @date 9.12.2021
 * @brief takes an arbitrary number of lines and sorts them
 * 
 * @detail reads an arbitrary number of lines from stdin
 *         forks twice and recursively calls itself
 *         splits up the read lines and passes them to the children
 *         children either exit and print input immediately with just one input
 *          or act as parents themselve
 *         parents read output of children and uses merge sort to produce the output for its parent
 *          or print to stdout
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/wait.h>
#include <stdbool.h>

/**
 * @brief performs dup2 on read/writePipe with stdin/stdout and closes the unsued ends
 * 
 * @detail replaces the read end of the readPipe with stdin
 *         replaces the write end of the writePipe with stdout 
 *          and closes the (unused) ends afterwards
 * 
 * @param readPipe pipe to replace pipe read end with stdin
 * @param writePipe pipe to replace pipe write end with stdout
 */
static void dup_pipes(int *readPipe, int *writePipe);

int main(int argc, char *argv[])
{
    int lines_size = 2;
    char **lines = malloc(sizeof(char *) * lines_size);
    int index = 0; // current index of lines to write the next input to
    int p = 1;

    // read from stdin
    char *line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, stdin)) != -1)
    {
        if (index == lines_size)
        {
            lines_size = pow(2, ++p);
            lines = realloc(lines, sizeof(char *) * lines_size);
        }
        lines[index] = malloc(sizeof(char) * len);
        strcpy(lines[index], line);
        index++;
    }
    free(line);

    if (index < 2)
    {
        // write to stdout and exit
        for (size_t i = 0; i < index; i++)
        {
            fprintf(stdout, "%s", lines[i]);
        }
        // free lines
        free(lines);
        fclose(stdout);
        exit(EXIT_SUCCESS);
    }

    /************* child 1 *************/

    // create pipe
    int pipe1c1fd[2]; // child reads, parent writes
    int pipe2c1fd[2]; // child writes, parent reads
    if ((pipe(pipe1c1fd) == -1) || (pipe(pipe2c1fd) == -1))
    {
        fprintf(stderr, "pipe failed: %s\n", strerror(errno));
    }

    pid_t pid1 = fork();

    switch (pid1)
    {
    case 0: // child

        // dup2 on pipe ends /replace with stdin and stdout
        dup_pipes(pipe1c1fd, pipe2c1fd);

        // call execlp (propgates dupped stdin and stdout)
        execlp(argv[0], argv[0], NULL);

        fprintf(stderr, "execlp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    case -1: // error
        fprintf(stderr, "forking failed %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    default: // parent
        break;
    }

    /************* child 2 *************/

    // create pipe
    int pipe1c2fd[2]; // child reads, parent writes
    int pipe2c2fd[2]; // child writes, parent reads
    if ((pipe(pipe1c2fd) == -1) || (pipe(pipe2c2fd) == -1))
    {
        fprintf(stderr, "pipe failed: %s\n", strerror(errno));
    }

    pid_t pid2 = fork();

    switch (pid2)
    {
    case 0: // child

        // dup2 on pipe ends /replace with stdin and stdout
        dup_pipes(pipe1c2fd, pipe2c2fd);

        // call execlp (propgates dupped stdin and stdout)
        execlp(argv[0], argv[0], NULL);

        fprintf(stderr, "execlp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    case -1: // error
        fprintf(stderr, "forking failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    default: // parent
        break;
    }

    FILE *writeEnd1 = fdopen(pipe1c1fd[1], "w");
    close(pipe1c1fd[0]); // unused read end

    FILE *readEnd1 = fdopen(pipe2c1fd[0], "r");
    close(pipe2c1fd[1]); // unused write end

    FILE *writeEnd2 = fdopen(pipe1c2fd[1], "w");
    close(pipe1c2fd[0]); // unused read end

    FILE *readEnd2 = fdopen(pipe2c2fd[0], "r");
    close(pipe2c2fd[1]); // unused write end

    // write split up input to resp child stdin
    for (size_t i = 0; i < index / 2; i++)
    {
        fprintf(writeEnd1, lines[i]);
        free(lines[i]);
        fprintf(writeEnd2, lines[(index / 2) + i]);
        free(lines[(index / 2) + i]);
    }
    if (index % 2 != 0)
    {
        fprintf(writeEnd2, lines[index - 1]);
        free(lines[index - 1]);
    }

    free(lines);
    fclose(writeEnd1);
    fclose(writeEnd2);

    // wait till children finished
    int status;
    wait(&status);
    if (status != EXIT_SUCCESS)
    {
        exit(status);
    }    
    wait(&status);
    if (status != EXIT_SUCCESS)
    {
        exit(status);
    }

    /************* sort *************/

    bool readF1 = true; // read line from F1
    bool readF2 = true; // read line from F2
    char *lineF1 = NULL;
    char *lineF2 = NULL;
    len = 0;
    ssize_t read_retvalF1;
    ssize_t read_retvalF2;

    while (true)
    {
        if (readF1)
        {
            read_retvalF1 = getline(&lineF1, &len, readEnd1);
        }
        if (readF2)
        {
            read_retvalF2 = getline(&lineF2, &len, readEnd2);
        }

        if ((read_retvalF2 == EOF) && (read_retvalF1 == EOF))
        {
            break;
        }
        else if ((read_retvalF1 == EOF) && (read_retvalF2 != EOF))
        {
            fprintf(stdout, lineF2);
            readF1 = false;
            readF2 = true;
        }
        else if ((read_retvalF2 == EOF) && (read_retvalF1 != EOF))
        {
            fprintf(stdout, lineF1);
            readF2 = false; 
            readF1 = true;
        }
        else
        {
            // compare lines and produce output
            int c = strcmp(lineF1, lineF2);
            if (c < 0) // lineF1 smaller
            {
                fprintf(stdout, lineF1);
                readF2 = false;
                readF1 = true;
            }
            if (c > 0) // lineF2 smaller
            {
                fprintf(stdout, lineF2);
                readF1 = false;
                readF2 = true;
            }
            if (c == 0) // both equal
            {
                fprintf(stdout, lineF1);
                fprintf(stdout, lineF2);
                readF1 = true;
                readF2 = true;
            }
        }
    }

    fclose(readEnd1);
    fclose(readEnd2);

    free(lineF1);
    free(lineF2);

    return EXIT_SUCCESS;
}

void dup_pipes(int *readPipe, int *writePipe)
{
    if (dup2(readPipe[0], STDIN_FILENO) == -1)
    {
        fprintf(stderr, "dup2 on stdin failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(readPipe[0]);
    close(readPipe[1]); // unused write end

    if (dup2(writePipe[1], STDOUT_FILENO) == -1)
    {
        fprintf(stderr, "dup2 on stdout failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(writePipe[1]);
    close(writePipe[0]); // unused read end
}