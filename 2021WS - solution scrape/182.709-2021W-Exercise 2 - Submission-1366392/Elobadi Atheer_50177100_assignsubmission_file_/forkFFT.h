/**
 * @file forkFFT.h
 * @author Atheer Elobadi (e01049225@student.tuwien.ac.at)
 * @brief contains the constants, variable types and function 
 *        declerations which are used in the program.
 * @date 11.12.2021 
 */
#ifndef FORKFFT_H
#define FORKFFT_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/wait.h>

#define PI 3.1415926

#define PUSH_FAILURE -1
#define PUSH_SUCCESS 0

#define INIT_FAILURE -1
#define INIT_SUCCESS 0

#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

#define CHILD_ODD 0
#define CHILD_EVEN 1

#define PIPE_SUCCESS 0
#define PIPE_FAILURE -1


typedef struct {
    int pipefd_down[2];
    int pipefd_up[2];
    __pid_t pid;
} child_t;

typedef struct {
    float real; 
    float img;
} complex_t;

/**
 * @brief Prints the usage message and exits with EXIT_FILURE
 * 
 * @param program_name the name of the program to be included int the message.
 */
void usage(char* program_name);


/**
 * @brief Convert string to a complex number
 * 
 * @param str the string which to convert
 * @return complex_t Null if failed. a complex number is returned otherwise.
 */
void strtoc(complex_t *c, char *str);

/**
 * @brief Applies the Butterfly operation. 
 * 
 * @param result the array of complex_t where the result will be stored
 * @param c_even the even complex number
 * @param c_odd the odd complex number
 * @param count the size of the array
 * @param fd_up the pipe coming from the child from which the even and odd numbers should be read. 
 */
void butterfly(complex_t *result, complex_t *c_even, complex_t *c_odd, int count, FILE *fd_up[]);

/**
 * @brief Initialize the pipes
 * 
 * @param down the pipe which goes down towards the children. 
 * @param up the pipe which goes up towards the parent
 * @return int return -1 on failure.
 */
int init_pipes(int down[2], int up[2]);

#endif
