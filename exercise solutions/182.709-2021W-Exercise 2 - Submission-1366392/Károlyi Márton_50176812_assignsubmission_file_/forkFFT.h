#ifndef FORK
#define FORK

#define PI 3.141592654


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <math.h>
#include <string.h>

// #include <errno.h>
// #include <assert.h>

typedef struct {
    char *buffer1;
    char *buffer2;
    size_t size1;
    size_t size2;
} dual_buffer_t;

typedef struct {
    int in[2];
    int out[2];
    FILE *read;
    FILE *write;
} dual_pipe_t;

typedef struct {

    dual_pipe_t child1;
    dual_pipe_t child2;
} pipes_t;

typedef struct {
    pid_t child1;
    pid_t child2;

} pid_pair_t;

typedef struct {
    float real;
    float imag;
} complex_t;


#endif