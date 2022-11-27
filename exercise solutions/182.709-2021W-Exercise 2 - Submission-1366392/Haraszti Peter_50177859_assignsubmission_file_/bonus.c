/**
 * @file bonus.c
 * @author Peter Haraszti (e12019844@tuwien.ac.at)
 * @date 2021-12-11
 * 
 * @brief Bonus Task of UE2
 * @details Bonus Task for Fork Fourier Transformation. A tree which shows how forkFFT forks is printed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "structs.h"

char *programname = NULL;

/**
 * @brief Exits and prints an error message
 * @details The program exits with EXIT_FAILURE: The message specified in the parameter is printed to stderr
 * @param char *message
 * @return void
 */
void exit_error(char *message)
{
    fprintf(stderr, "%s: %s\n", programname, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints the branches
 * @details Prints a number of empty characters depending on n aswell as / and \
 * @param int n 
 * @return void
 */
void print_branches(int n)
{
    for (int i = 0; i < n; i++)
        fprintf(stdout, "      "); // print seven chars per float value
    fprintf(stdout, "/");
    for (int i = 0; i < n; i++)
        fprintf(stdout, "      "); // print six chars per float value
    fprintf(stdout, "\\");
    for (int i = 0; i < n; i++)
        fprintf(stdout, "      "); // print seven chars per float value
    fprintf(stdout, "\n");
}

/**
 * @brief Inits the child processes and forks
 * @details Two pipes are created, one for writing to the child process (stdinpipe) and one for reading from the child process (stdoutpipe)
 *          Then the program is forked. The pipes are set up, and then execlp is called which starts the child processes.
 *          The pid and the address of the pipes are stored in the struct child_info info.
 * @param child_info *info
 * @return void
 */
void init_child(child_info *info)
{
    int stdinpipe[2];
    int stdoutpipe[2];

    if (pipe(stdinpipe) == -1 || pipe(stdoutpipe) == -1)
        exit_error("Unable to open pipes!");

    pid_t pid = fork();
    info->pid = pid;

    switch (pid)
    {
    case -1:
        exit_error("Unable to fork!");
        break;
    case 0:                   // child
        close(stdinpipe[1]);  // close write end of stdin pipe
        close(stdoutpipe[0]); // close read end of stdout pipe

        if (dup2(stdinpipe[0], STDIN_FILENO) == -1)
            exit_error("Failed to redirect stdin!");

        if (dup2(stdoutpipe[1], STDOUT_FILENO) == -1)
            exit_error("Failed to redirect stdout!");

        close(stdinpipe[0]); // close pipes
        close(stdoutpipe[1]);

        if (execlp(programname, programname, NULL) == -1)
            exit_error("Failed to start child program!");
        break;

    default: // parent
        close(stdinpipe[0]);
        close(stdoutpipe[1]);
        info->read_end = stdoutpipe[0]; // The read end of the pipe to which the child redirects stdout is where the parent can read
        info->write_end = stdinpipe[1]; // The write end of the pipe to which the child redirects stdin is where the parent can write
        break;
    }
}

/**
 * @brief Writes an array of floats to the child
 * @details The write end of the pipe of the child process specified in "info" is opened using fdopen. 
 *          The contents of the array passed in "values" is written into the pipe. After that, the stream is closed.
 * @param float *values
 * @param int values_len
 * @param child_info info
 * @return void
 */
void write_to_child(float *values, int values_len, child_info info)
{
    FILE *fp;
    fp = fdopen(info.write_end, "w");

    if (fp == NULL)
        exit_error("Cannot write to pipe!");
    for (int i = 0; i < values_len; i++)
    {
        fprintf(fp, "%f\n", values[i]);
    }
    fclose(fp);
}

/**
 * @brief Subtrees of the child processes are printed
 * @details The output of the two child processes is read over a pipe line by line. 
 * The newline characters are removed and the two lines of the two children are concatenated and written to stdout.
 * @param evenchild 
 * @param oddchild 
 * @return void
 */
void print_children(child_info evenchild, child_info oddchild)
{
    FILE *evenfp, *oddfp;
    evenfp = fdopen(evenchild.read_end, "r");
    if (evenfp == NULL)
        exit_error("Failed opening read end of evenchild!");
    oddfp = fdopen(oddchild.read_end, "r");
    if (oddfp == NULL)
        exit_error("Failed opening read end of oddchild!");

    size_t bufsize_even = 64;
    size_t bufsize_odd = 64;
    char *input_even, *input_odd;

    input_even = (char *)malloc(bufsize_even * sizeof(char));
    if (input_even == NULL)
        exit_error("Unable to allocate memory for reading the result of evenchild!");
    input_odd = (char *)malloc(bufsize_odd * sizeof(char));
    if (input_odd == NULL)
        exit_error("Unable to allocate memory for reading the result of oddchild!");

    while (getline(&input_even, &bufsize_even, evenfp) != -1 && getline(&input_odd, &bufsize_odd, oddfp) != -1)
    {
        input_even[strlen(input_even) - 1] = '\0';
        input_odd[strlen(input_odd) - 1] = '\0';
        fprintf(stdout, "%s", input_even);
        fprintf(stdout, "%s\n", input_odd);
    }

    free(input_even);
    free(input_odd);
    fclose(evenfp);
    fclose(oddfp);
}

/**
 * @brief Main Method of the Bonus Task of UE2
 * @details The Bonus Task works in a similar way as forkFFT. 
 * Instead of the results of the Fourier Transformation, a tree which shows how the program forks is printed. 
 * @param argc 
 * @param argv 
 * @return EXIT_SUCCESS if everything went well 
 */
int main(int argc, char **argv)
{
    programname = argv[0];

    if (argc > 1)
        exit_error("bonus takes no arguments!");

    // Allocate memory for reading from stdin
    size_t bufsize = 32;
    char *input;
    input = (char *)malloc(32 * sizeof(char)); //dynamically allocated memory where the input is stored
    if (input == NULL)
        exit_error("Unable to allocate memory for reading the input!");

    // Allocate memory for storing the float values in an array
    float *floatarr;
    int arrsize = 8;
    floatarr = (float *)calloc(arrsize, sizeof(float));
    if (floatarr == NULL)
        exit_error("Unable to allocate memory for the array of input floats!");

    // Read and store the float values
    int n = 0;
    while (getline(&input, &bufsize, stdin) != EOF)
    {
        float val = strtof(input, NULL);
        if (n >= arrsize)
        {
            arrsize = arrsize * 2;
            floatarr = realloc(floatarr, arrsize * sizeof(float));
            if (floatarr == NULL)
                exit_error("Unable to reallocate memory for the array of input floats!");
        }
        floatarr[n] = val;
        n++;
    }

    free(input);

    // Base case, if the array consists of only one element or the input is empty
    if (n <= 1)
    {
        fprintf(stdout, "     %f     \n", floatarr[0]);
        free(floatarr);
        exit(EXIT_SUCCESS);
    }

    if (n % 2 == 1)
    {
        free(floatarr);
        exit_error("Got array with an even length!");
    }

    for (int i = 0; i < n; i++)
        fprintf(stdout, "    "); // print four chars per float value

    fprintf(stdout, "FFT(");
    for (int i = 0; i < n; i++)
    {
        fprintf(stdout, "%f", floatarr[i]);
        if (i != n - 1)
            fprintf(stdout, ", ");
    }
    fprintf(stdout, ")");
    for (int i = 0; i < n; i++)
        fprintf(stdout, "    "); // print four chars per float value
    fprintf(stdout, "\n");

    // print the branches to which the subtrees of the children will connect
    print_branches(n);

    // Split the array
    float *odd_arr;
    float *even_arr;
    odd_arr = (float *)calloc(n / 2, sizeof(float));
    even_arr = (float *)calloc(n / 2, sizeof(float));

    int odd_i = 0;
    int even_i = 0;

    for (int i = 0; i < n; i++)
    {
        if (i % 2 == 0)
            even_arr[even_i++] = floatarr[i];

        if (i % 2 == 1)
            odd_arr[odd_i++] = floatarr[i];
    }

    free(floatarr);

    // Fork, spawn children
    child_info even_child;
    child_info odd_child;
    init_child(&even_child);
    init_child(&odd_child);

    // Write splitted arrays to the children
    write_to_child(even_arr, even_i, even_child);
    write_to_child(odd_arr, odd_i, odd_child);

    free(odd_arr);
    free(even_arr);

    // Wait for the children to finish their calculations
    int status_even;
    int status_odd;
    waitpid(even_child.pid, &status_even, 0);
    waitpid(odd_child.pid, &status_odd, 0);
    if (WEXITSTATUS(status_even) != EXIT_SUCCESS || WEXITSTATUS(status_odd) != EXIT_SUCCESS)
    {
        close(even_child.read_end);
        close(odd_child.read_end);
        exit_error("Failed waiting on child, it is probably dead.");
    }

    // print the subtrees retrieved from the children
    print_children(even_child, odd_child);

    return EXIT_SUCCESS;
}