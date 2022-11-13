#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

/**
 * @brief This method outputs the synopsis for the command
 * 
 * @param error an error to be printed
 */
void synopsis(char *error)
{
    fputs(error, stderr);
    fputs("SYNOPSIS\n\t\tforksort\n", stderr);
    exit(EXIT_FAILURE);
}

/**
 * @brief This methods removes the last character from a string, if it is '\n' by replacing it with '\0'
 * 
 * @param string The string of which the last
 * @param len The initial length of the string
 */
void removeNewline(char *string, int len)
{
    if (string[len - 1] == '\n')
    {
        string[len - 1] = '\0';
    }
}

/**
 * @brief Main function of the program. Performs a line-wise sorting of stdio by repeatedly calling itself (multi-process mergesort)
 * 
 * @param argc argument count
 * @param argv arguments
 * @return int 
 */
int main(int argc, char **argv)
{
    if (argc > 1)
    {
        synopsis("Too many arguments given. \n");
    }
    char *line = NULL;
    char *line2 = NULL;
    size_t len;
    ssize_t chars_read = 0;

    if ((chars_read = getline(&line, &len, stdin)) == -1)
    {
        // zero lines. nothing to sort
        free(line);
        exit(EXIT_SUCCESS);
    }
    removeNewline(line, chars_read);

    if ((chars_read = getline(&line2, &len, stdin)) == -1)
    {
        // only one line -> return that line
        fprintf(stdout, "%s\n", line);
        free(line);
        free(line2);
        exit(EXIT_SUCCESS);
    }
    removeNewline(line2, chars_read);

    int inpipe1[2];
    int outpipe1[2];

    if (pipe(inpipe1) == -1 || pipe(outpipe1) == -1)
    {
        free(line);
        free(line2);
        exit(EXIT_FAILURE);
    }

    // now do forking
    pid_t forkid = fork();
    if (forkid == -1)
    {
        free(line);
        free(line2);
        // fork error
        exit(EXIT_FAILURE);
    }
    else if (forkid == 0)
    {
        // child process
        close(inpipe1[1]);
        close(outpipe1[0]);

        dup2(inpipe1[0], STDIN_FILENO);
        dup2(outpipe1[1], STDOUT_FILENO);

        execlp("./forksort", "forksort", NULL);
        perror("exec not working");
    }
    close(inpipe1[0]);
    close(outpipe1[1]);

    int inpipe2[2];
    int outpipe2[2];

    if (pipe(inpipe2) == -1 || pipe(outpipe2) == -1)
    {
        free(line);
        free(line2);
        exit(EXIT_FAILURE);
    }

    pid_t forkid1 = fork();
    if (forkid1 == -1)
    {
        // fork error
        free(line);
        free(line2);
        exit(EXIT_FAILURE);
    }
    else if (forkid1 == 0)
    {
        // child process
        close(inpipe2[1]);
        close(outpipe2[0]);

        dup2(inpipe2[0], STDIN_FILENO);
        dup2(outpipe2[1], STDOUT_FILENO);

        execlp("./forksort", "forksort", NULL);
        perror("exec not working");
    }

    close(inpipe2[0]);
    close(outpipe2[1]);

    char newline = '\n';

    write(inpipe1[1], line, strlen(line));
    write(inpipe1[1], &newline, sizeof(newline));

    write(inpipe2[1], line2, strlen(line2));
    write(inpipe2[1], &newline, sizeof(newline));

    int toggle = 1;
    while ((chars_read = getline(&line, &len, stdin)) != -1)
    {
        removeNewline(line, chars_read);
        if (toggle)
        {

            write(inpipe1[1], line, strlen(line));
            write(inpipe1[1], &newline, sizeof(newline));
            toggle = 0;
        }
        else
        {
            write(inpipe2[1], line, strlen(line));
            write(inpipe2[1], &newline, sizeof(newline));
            toggle = 1;
        }
    }
    free(line);
    free(line2);
    close(inpipe1[1]);
    close(inpipe2[1]);

    // wait for child processes to check startcode
    int status = 0;

    // question: merging this two ifs into one if would not be defined by c standard, or?
    if (waitpid(forkid, &status, 0) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (status != 0)
    {
        exit(EXIT_FAILURE);
    }
    if (waitpid(forkid1, &status, 0) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (status != 0)
    {
        exit(EXIT_FAILURE);
    }

    FILE *pipe1out = fdopen(outpipe1[0], "r");
    FILE *pipe2out = fdopen(outpipe2[0], "r");
    size_t l1 = 0, l2 = 0;
    int l1res = getline(&line, &l1, pipe1out);
    int l2res = getline(&line2, &l2, pipe2out);

    while (l1res != EOF || l2res != EOF)
    {
        if (l1res > 0)
            removeNewline(line, l1res);
        if (l2res > 0)
            removeNewline(line2, l2res);
        if (l1res == EOF)
        {
            fprintf(stdout, "%s\n", line2);
            l2res = getline(&line2, &l2, pipe2out);
        }
        else if (l2res == EOF)
        {
            fprintf(stdout, "%s\n", line);
            l1res = getline(&line, &l1, pipe1out);
        }
        else if (strcmp(line, line2) < 0)
        {
            fprintf(stdout, "%s\n", line);
            l1res = getline(&line, &l1, pipe1out);
        }
        else
        {
            fprintf(stdout, "%s\n", line2);
            l2res = getline(&line2, &l2, pipe2out);
        }
    }

    fclose(pipe1out);
    fclose(pipe2out);
    close(outpipe2[0]);
    close(outpipe2[0]);
    free(line);
    free(line2);

    return 0;
}
