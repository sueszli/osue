/**
 * @file forksort.c
 * @author Philipp Nemec 11907551
 * @date 11.12.2021
 *
 * @brief A program that takes multiple lines as input and sorts them by using a recursive variant of merge sort.
 *
 * @details The program reads its input from stdin and ends when an EOF is encountered. The program accepts any number of lines and
 * sorts the lines by calling itself. If the input consist of only 1 line then this line is written to stdout and the program exits with status EXIT_SUCCESS.
 * Otherwise the input is split into two parts. If the input is odd one of the parts will have one line more than the other.
 * The program then recursively executes itself in two child processes, one for each of the two parts. It uses two unnamed pipes per child to redirect stdin and stdout.
 * The first part is written to stdin of one child and then the second part is written to stdin of the other child. Afterwards the respective sorted lines from each child's stdout is read.
 * The two child processes run simultaneously. The program terminates with exit status EXIT_FAILURE if the exit status of any of the two child processes is not EXIT_SUCCESS.
 * At the end the sorted parts from the two child processes are merged and written to stdout. At each step the next line of both parts are compared and the smaller one is written to stdout,
 * such that the lines are written in alphabetical order. The sorting is case sensitive.
 **/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PIPE_AMOUNT 4
#define PIPE_1_R 0
#define PIPE_1_W 1
#define PIPE_2_R 2
#define PIPE_2_W 3

static int pipes[PIPE_AMOUNT][2]; //This global variable holds all pipes with their individual read/write end in an array because it is used almost everywhere in the program.

/**
 * @brief The function reads from each child's read pipe into an array of strings and after comparing the entries it writes the results to stdout.
 * 
 * @param counter1 A pointer which stores the amount of lines written to the first child.
 * @param counter2 A pointer which stores the amount of lines written to the second child.
 * @return The return code of the function (0 is ok and -1 is an error).
 */
static int readPipesAndSort(int counter1, int counter2)
{
    FILE *pipe1 = fdopen(pipes[PIPE_1_R][0], "r");
    if (pipe1 == NULL)
    {
        return -1;
    }
    char **inputPipe1 = malloc(sizeof(char *) * counter1);
    if (inputPipe1 == NULL)
    {
        return -1;
    }
    char *line1 = NULL;
    size_t len1 = 0;
    for (int i = 0; i < counter1; i++)
    {
        getline(&line1, &len1, pipe1);
        inputPipe1[i] = line1;
        inputPipe1[i][strlen(inputPipe1[i]) - 1] = '\0'; //Without a null terminator (at the end of the entire read input) strcmp runs into problems.
        line1 = NULL;
    }
    free(line1);
    fclose(pipe1);
    FILE *pipe2 = fdopen(pipes[PIPE_2_R][0], "r");
    if (pipe2 == NULL)
    {
        return -1;
    }
    char **inputPipe2 = malloc(sizeof(char *) * counter2);
    if (inputPipe2 == NULL)
    {
        return -1;
    }
    char *line2 = NULL;
    size_t len2 = 0;
    for (int i = 0; i < counter2; i++)
    {
        getline(&line2, &len2, pipe2);
        inputPipe2[i] = line2;
        inputPipe2[i][strlen(inputPipe2[i]) - 1] = '\0'; //Without a null terminator (at the end of the entire read input) strcmp runs into problems.
        line2 = NULL;
    }
    free(line2);
    fclose(pipe2);
    int counter1back = 0;
    int counter2back = 0;
    while (counter1back != counter1 && counter2back != counter2) //Write winner (alphabetically) to stdout.
    {
        if (strcmp(inputPipe1[counter1back], inputPipe2[counter2back]) > 0)
        {
            fprintf(stdout, "%s\n", inputPipe2[counter2back]);
            free(inputPipe2[counter2back]);
            counter2back++;
        }
        else
        {
            fprintf(stdout, "%s\n", inputPipe1[counter1back]);
            free(inputPipe1[counter1back]);
            counter1back++;
        }
    }
    while (counter1back != counter1 || counter2back != counter2) //Write rest to stdout.
    {
        if (counter1back == counter1)
        {
            fprintf(stdout, "%s\n", inputPipe2[counter2back]);
            free(inputPipe2[counter2back]);
            counter2back++;
        }
        else
        {
            fprintf(stdout, "%s\n", inputPipe1[counter1back]);
            free(inputPipe1[counter1back]);
            counter1back++;
        }
    }
    free(inputPipe1);
    free(inputPipe2);
    return 0;
}

/**
 * @brief The function writes the input divided by half (second child gets one more if odd) line by line into each child's write pipe.
 * 
 * @param inputSize The amount of lines from the input.
 * @param input An array of strings which holds the entire input.
 * @param counter1 A pointer which stores the amount of lines written to the first child.
 * @param counter2 A pointer which stores the amount of lines written to the second child.
 * @return The return code of the function (0 is ok and -1 is an error).
 */
static int writePipes(int inputSize, char **input, int *counter1, int *counter2)
{
    int i = 0;
    for (; i < inputSize / 2; i++)
    {
        if (write(pipes[PIPE_1_W][1], input[i], strlen(input[i])) == -1)
        {
            return -1;
        }
        free(input[i]);
        (*counter1)++;
    }
    for (; i < inputSize; i++)
    {
        if (write(pipes[PIPE_2_W][1], input[i], strlen(input[i])) == -1)
        {
            return -1;
        }
        free(input[i]);
        (*counter2)++;
    }
    return 0;
}

/**
 * @brief The function individually redirects stdout and stdin for each child and closes all pipes at the end.
 * 
 * @param first Indicates if the pipe for the first child or the pipe for the second child should be redirected.
 * @return The return code of the function (0 is ok and -1 is an error).
 */
static int redirectPipes(bool first)
{
    int read, write;
    if (first == true)
    {
        read = PIPE_1_R;
        write = PIPE_1_W;
    }
    else
    {
        read = PIPE_2_R;
        write = PIPE_2_W;
    }
    if (dup2(pipes[read][1], STDOUT_FILENO) == -1)
    {
        return -1;
    }
    if (dup2(pipes[write][0], STDIN_FILENO) == -1)
    {
        return -1;
    }
    for (int i = 0; i < PIPE_AMOUNT; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    return 0;
}

/**
 * @brief The function forks the handed over process, if it is a child it additionally redirects its pipes and executes the program on it.
 * 
 * @param p A pointer to the process id which should be forked.
 * @param first Indicates if the pipe for the first child or the pipe for the second child should be redirected.
 * @param name The program name (from argv[0]).
 * @return The return code of the function (0 is ok and -1 is an error).
 */
static int forkAndExec(int *p, bool first, char *name)
{
    *p = fork();
    if (*p == -1)
    {
        return -1;
    }
    if (*p == 0) //The process was detected as a child.
    {
        if (redirectPipes(first) == -1)
        {
            return -1;
        }
        if (execlp("./forksort", name, NULL) == -1) //The child arguments are NULL because communication is done over pipes.
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief The start of the program.
 * 
 * @details First of all the arguments are checked. After this the entire input is temporarily stored in an array of strings to determine its amount of lines.
 * If the amount is greater than 1 the program continues, otherwise a single line is written to stdout directly or zero lines triggers an error.
 * Now the 4 pipes and the 2 childs are created. Before the childs get to start their recursion they get their stdout and stdin redirected to the respective pipes.
 * After this the divided input is written to each child and the program waits for all childs to finish. At the end the results are read from each child, sorted and written to stdout.
 * 
 * @param argc The argument counter of the program.
 * @param argv The argument values of the program.
 * @return The exit code of the program.
 */
int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    char **input = malloc(sizeof(char *));
    if (input == NULL)
    {
        fprintf(stderr, "%s: malloc failed!\n", argv[0]);
        return EXIT_FAILURE;
    }
    int inputSize = 0;
    char *line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, stdin)) != -1)
    {
        input = realloc(input, sizeof(char *) * (inputSize + 1));
        if (input == NULL)
        {
            fprintf(stderr, "%s: realloc failed!\n", argv[0]);
            return EXIT_FAILURE;
        }
        input[inputSize] = line;
        inputSize++;
        line = NULL;
    }
    free(line);
    if (inputSize == 0)
    {
        fprintf(stderr, "%s: empty file!\n", argv[0]);
        free(input);
        return EXIT_FAILURE;
    }
    if (inputSize == 1)
    {
        printf("%s\n", input[0]);
		free(input[0]);
        free(input);
        return EXIT_SUCCESS;
    }
    if ((pipe(pipes[PIPE_1_R]) == -1) || (pipe(pipes[PIPE_1_W]) == -1) || (pipe(pipes[PIPE_2_R]) == -1) || (pipe(pipes[PIPE_2_W]) == -1))
    {
        fprintf(stderr, "%s: pipe failed!\n", argv[0]);
        free(input);
        return EXIT_FAILURE;
    }
    int process1, process2;
    if (forkAndExec(&process1, true, argv[0]) == -1)
    {
        fprintf(stderr, "%s: fork, execlp or dup2 (process1) failed!\n", argv[0]);
        free(input);
        return EXIT_FAILURE;
    }
    if (forkAndExec(&process2, false, argv[0]) == -1)
    {
        fprintf(stderr, "%s: fork, execlp or dup2 (process2) failed!\n", argv[0]);
        free(input);
        return EXIT_FAILURE;
    }
    close(pipes[PIPE_1_R][1]); //Close unnecessary pipes.
    close(pipes[PIPE_1_W][0]);
    close(pipes[PIPE_2_R][1]);
    close(pipes[PIPE_2_W][0]);
    int counter1 = 0, counter2 = 0;
    if (writePipes(inputSize, input, &counter1, &counter2) == -1)
    {
        fprintf(stderr, "%s: write failed!\n", argv[0]);
        free(input);
        return EXIT_FAILURE;
    }
    free(input);
    close(pipes[PIPE_1_W][1]); //Close write pipes.
    close(pipes[PIPE_2_W][1]);
    int status;
    if (waitpid(process1, &status, 0) == -1)
    {
        fprintf(stderr, "%s: waitpid (process1) failed!\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (WEXITSTATUS(status) != EXIT_SUCCESS)
    {
        fprintf(stderr, "%s: process1 failed!\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (waitpid(process2, &status, 0) == -1)
    {
        fprintf(stderr, "%s: waitpid (process2) failed!\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (WEXITSTATUS(status) != EXIT_SUCCESS)
    {
        fprintf(stderr, "%s: process2 failed!\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (readPipesAndSort(counter1, counter2) == -1)
    {
        fprintf(stderr, "%s: read (fdopen or malloc) failed!\n", argv[0]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
