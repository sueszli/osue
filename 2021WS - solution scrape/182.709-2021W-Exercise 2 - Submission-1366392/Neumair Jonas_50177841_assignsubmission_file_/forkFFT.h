/**
 * @file forkFFT.h
 * @author Jonas Neumair <e11911064@student.tuwien.ac.at>
 * @date 10.12.2021
 *
 * @brief Header file for forkFFT.c.
 * 
 * This program 
 **/
#include <stdlib.h>
#ifndef _FORKFFT_H_
#define _FORKFFT_H_
#define MAX_LINE_LENGTH (255)
#define PI (3.141592654)

typedef struct child {
    int read_pipe;
    int write_pipe;
    pid_t pid;
} child_t;

typedef struct ComplexNumber {
    float real;
    float imaginary;
} complex_t;

#endif
