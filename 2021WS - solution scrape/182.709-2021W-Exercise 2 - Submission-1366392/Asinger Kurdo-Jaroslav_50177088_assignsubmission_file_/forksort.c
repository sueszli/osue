#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * @brief programe name - global variable useful for errors and exec
 */
char *progname = NULL;

/**
 * @brief prints Synopsis and exists program
 * @param message description of error regarding arguments
 */
void usage(char *message)
{
    fprintf(stderr, "%s\nSynopsis: %s", message, progname);
    exit(EXIT_FAILURE);
}

/**
 * @brief saves child process ID and pipe FDs
 */
typedef struct
{
    pid_t childId;
    int reader;
    int writer;
} child_p;

/**
 * @brief handles fork and pipe
 * 
 * @param child struct to save child process ID and pipe FDs
 */
void fork_pipe(child_p *child)
{

    int pipeStdIn[2];
    int pipeStdOut[2];

    int pin = pipe(pipeStdIn);
    if (pin == -1)
    {
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int pout = pipe(pipeStdOut);
    if (pout == -1)
    {
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    child->childId = fork();

    switch (child->childId)
    {
    case -1:
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
        break;
    case 0:
        close(pipeStdIn[1]);
        dup2(pipeStdIn[0], STDIN_FILENO);
        close(pipeStdOut[0]);
        dup2(pipeStdOut[1], STDOUT_FILENO);
        execlp(progname, progname, NULL);
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
        break;
    default:
        close(pipeStdIn[0]);
        close(pipeStdOut[1]);
        child->reader = pipeStdIn[1];
        child->writer = pipeStdOut[0];
        break;
    }
}

int main(int argc, char *argv[])
{
    progname = argv[0];

    /**
     * @brief there should be no arguments (except for the program name)
     * 
     */
    if (argc != 1)
    {
        usage("Wrong number of Arguments");
    }

    char **toSort = NULL;
    if ((toSort = malloc(2 * sizeof(char *))) == NULL)
    {
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    char *input = NULL;
    size_t size = NULL;
    size_t length = 0;
    int i = 0;
    int toSortSize = 2;

    /**
     * @brief every line read from stdin will be written into toSort 
     * 
     */
    while (-1 != (length = getline(&input, &size, stdin)))
    {
        /**
         * @brief newlines should be disregarded for the sort
         * 
         */
        if (input[length - 1] == '\n')
        {
            length--;
        }
        if ((toSort[i] = malloc(sizeof(char) * (length + 1))) == NULL)
        {
            fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
            exit(EXIT_FAILURE);
        }
        strncpy(toSort[i], input, length);
        toSort[i][length] = '\0';
        i++;
        /**
         * @brief double the size of toSort whenever the last position has been used
         * 
         */
        if (i >= toSortSize)
        {
            toSortSize *= 2;
            if ((toSort = realloc(toSort, toSortSize * sizeof(char *))) == NULL)
            {
                fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
                int j;
                for (j = 0; j < i; j++)
                {
                    free(toSort[j]);
                }
                free(toSort);
                free(input);
                exit(EXIT_FAILURE);
            }
        }
    }
    free(input);

    /**
     * @brief trivial case: no input
     * 
     */
    if (i == 0)
    {
        exit(EXIT_SUCCESS);
    }

    /**
     * @brief trivial case: exactly one input line
     * 
     */
    if (i == 1)
    {
        fprintf(stdout, "%s\n", toSort[0]);
        exit(EXIT_SUCCESS);
    }

    /**
     * @brief fork 2 times (one child for the left half, the other for the right half)
     * 
     */
    child_p childLeft = {0};
    fork_pipe(&childLeft);
    child_p childRight = {0};
    fork_pipe(&childRight);

    /**
     * @brief left half - write into child's read input (identify by saved FD)
     * 
     */
    FILE *writeinleft = NULL;
    if ((writeinleft = fdopen(childLeft.reader, "w")) == NULL)
    {
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int j;
    for (j = 0; j < (i / 2); j++)
    {
        fprintf(writeinleft, "%s\n", toSort[j]);
    }
    fclose(writeinleft);

    /**
     * @brief right half - write into child's read input (identify by saved FD)
     * 
     */
    FILE *writeinright = NULL;
    if ((writeinright = fdopen(childRight.reader, "w")) == NULL)
    {
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    for (; j < i; j++)
    {
        fprintf(writeinright, "%s\n", toSort[j]);
    }
    fclose(writeinright);

    for (j = 0; j < i; j++)
    {
        free(toSort[j]);
    }
    free(toSort);

    /**
     * @brief preparations for reading (merging)
     * 
     */
    FILE *readfromleft = NULL;
    FILE *readfromright = NULL;
    if ((readfromleft = fdopen(childLeft.writer, "r")) == NULL)
    {
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if ((readfromright = fdopen(childRight.writer, "r")) == NULL)
    {
        fprintf(stderr, "Error at %s: %s", progname, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *inputLeft = NULL;
    char *inputRight = NULL;
    size_t sizeLeft = NULL;
    size_t sizeRight = NULL;
    size_t lengthLeft = 0;
    size_t lengthRight = 0;
    lengthLeft = getline(&inputLeft, &sizeLeft, readfromleft);
    lengthRight = getline(&inputRight, &sizeRight, readfromright);

    /**
     * @brief choose the correct line of the two files as long as both half still have lines
     * 
     */
    while ((-1 != lengthLeft) && (-1 != lengthRight))
    {
        int comp = strcmp(inputLeft, inputRight);
        if (comp > 0)
        {
            fprintf(stdout, "%s", inputRight);
            lengthRight = getline(&inputRight, &sizeRight, readfromright);
        }
        else
        {
            fprintf(stdout, "%s", inputLeft);
            lengthLeft = getline(&inputLeft, &sizeLeft, readfromleft);
        }
    }

    /**
     * @brief the remaining lines are already sorted
     * 
     */
    while (-1 != lengthLeft)
    {
        fprintf(stdout, "%s", inputLeft);
        lengthLeft = getline(&inputLeft, &sizeLeft, readfromleft);
    }
    while (-1 != lengthRight)
    {
        fprintf(stdout, "%s", inputRight);
        lengthRight = getline(&inputRight, &sizeRight, readfromright);
    }
    fclose(readfromleft);
    fclose(readfromright);
    free(inputLeft);
    free(inputRight);

    int status = 0;
    waitpid(childLeft.childId, &status, 0);
    status = 0;
    waitpid(childRight.childId, &status, 0);

    exit(EXIT_SUCCESS);
}