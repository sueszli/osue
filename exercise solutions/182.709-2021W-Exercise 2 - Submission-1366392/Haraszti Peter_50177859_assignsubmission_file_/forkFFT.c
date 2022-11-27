/**
 * @file forkFFT.c
 * @author Peter Haraszti (e12019844@tuwien.ac.at)
 * @date 2021-11-22
 * 
 * @brief Fork Fourier Transform
 * @details Program that implements the Cooley-Tukey Fast Fourier Transform algorithm. The concepts of forking and pipes are used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

#include "structs.h"

#define PI 3.141592654

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
 * @brief Reads the result of a child process into the arrays realres and imres
 * @details The read end of the pipe of the child process specified in "child" is opened using fdopen.
 *          The content is read line by line. The real and imaginary values of the results are parsed and stored in the arrays realres and imres.
 *          In the end, the resources are cleaned up.
 * @param int maxlen
 * @param child_info child
 * @param float *realres
 * @param float *imres
 * @return void
 */
void read_childresult(int maxlen, child_info child, float *realres, float *imres)
{
    FILE *fp;
    fp = fdopen(child.read_end, "r");
    if (fp == NULL)
        exit_error("Failed opening read end of child!");

    size_t bufsize = 32;
    char *input;
    input = (char *)malloc(bufsize * sizeof(char));
    if (input == NULL)
        exit_error("Unable to allocate memory for reading the result of child!");

    int i = 0;

    while (i < maxlen)
    {
        getline(&input, &bufsize, fp);
        char *endptr;
        float real = strtof(input, &endptr);
        float im = strtof(endptr, NULL);
        realres[i] = real;
        imres[i] = im;
        i++;
    }

    free(input);
    fclose(fp);
}

/**
 * @brief Applies the butterfly operations to the results of two child processes
 * @details The results of the child processes are read. After that, the butterfly operation is performed as described in the assignment.
 *          Finally, the result stored in result_real and result_im is printed to stdout
 * @param int n
 * @param child_info evenchild
 * @param child_info oddchild
 * @return void
 */
void butterfly(int n, child_info evenchild, child_info oddchild)
{ // Read from children
    float even_real[n / 2];
    float even_im[n / 2];
    read_childresult(n / 2, evenchild, even_real, even_im);

    float odd_real[n / 2];
    float odd_im[n / 2];
    read_childresult(n / 2, oddchild, odd_real, odd_im);

    float result_real[n];
    float result_im[n];

    for (int k = 0; k < n / 2; k++) // Do butterfly calculations
    {
        float x = (-1) * ((2 * PI) / n) * k;
        result_real[k] = even_real[k] + (cosf(x) * odd_real[k]) - (sinf(x) * odd_im[k]);
        result_im[k] = even_im[k] + (cosf(x) * odd_im[k]) + (sinf(x) * odd_real[k]);

        result_real[k + n / 2] = even_real[k] - (cosf(x) * odd_real[k]) + (sinf(x) * odd_im[k]);
        result_im[k + n / 2] = even_im[k] - (cosf(x) * odd_im[k]) - (sinf(x) * odd_real[k]);
    }

    for (int i = 0; i < n; i++) // Print result
    {
        fprintf(stdout, "%f %f*i\n", result_real[i], result_im[i]);
    }
}

/**
 * @brief Main function of forkFFT
 * @details First, the input values are read from stdin and stored in the dynamically sized array floatarr.
 *          If the array consists of only one or zero elements, the program terminates. Otherwise the array is split into 
 *          an array with the elements of even indices and an array with the elements of odd indices. 
 *          Two children are spawned and the program forkes. The array with the even elements is written to one child, the one with the odd elements to the other child.
 *          The code waits for the children to finish and reads their results. The results are merged and printed using the function butterfly()
 * @param int argc
 * @param char **argv
 * @return void
 */
int main(int argc, char **argv)
{
    programname = argv[0];

    if (argc > 1)
        exit_error("forkFFT takes no arguments!");

    // Allocate memory for reading from stdin
    size_t bufsize = 32;
    size_t line;
    char *input;
    input = (char *)malloc(bufsize * sizeof(char)); //dynamically allocated memory where the input is stored
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
    while ((line = getline(&input, &bufsize, stdin)) != EOF)
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
        fprintf(stdout, "%f\n", floatarr[0]);
        free(floatarr);
        exit(EXIT_SUCCESS);
    }

    if (n % 2 == 1)
    {
        free(floatarr);
        exit_error("Got array with an even length!");
    }

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

    // reads results of children, butterflies them and prints them
    butterfly(n, even_child, odd_child);

    return EXIT_SUCCESS;
}
