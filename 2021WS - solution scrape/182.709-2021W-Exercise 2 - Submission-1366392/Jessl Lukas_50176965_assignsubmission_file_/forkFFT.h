/*
@autor: Jessl Lukas 01604985
@modulename: forkFFT.h
@created 20.11.2021


@brief: This programm takes an input, splits it into an odd and even part. With those two parts (they need to have the same length)
we will use a fast fourier transformation. We programm the fast fourier transformation recursivley after the "Cooley-Tukey" Algorithm. */

#ifndef FORKFFT_H
#define FORKFFT_H

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>

#define PI 3.141592654

char* readfromCommandoLine(char** argv);
void closeallusedMemory(char* even, char* odd);
void inputtofloatcomplex(float** even, float** odd, char* eveninput, char* oddinput);
void ctfft(float** even, float** odd, int length);
void closePipes3(int pipefd1, int pipefd2, int pipefd3);
void closePipes2(int pipefd1, int pipefd2);
 
#endif
