/**
 * @file datatypes.h
 * @author Lukas Fink 11911069
 * @date 03.12.2021
 *  
 * @brief datatypes Module
 *
 * contains datatypes required for the program
 **/

#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>


/** coord struct. 
 * @brief Contains coordinates (x,y)
 */
struct coord
{
    float x;
    float y;
};

void printCoordArray(FILE *stream, struct coord *arr, int size);

int readFromChild(struct coord arr[2], FILE *input);

double arithmeticMeanX(struct coord *arr, int size);

int countLEG(struct coord *input, int inputSize, double mean, char l_e_g);

int divide(struct coord *lessEq, struct coord *greater, struct coord *input, int inputSize, double mean);

int shortestDistanceBetween(struct coord closestBetween[2], struct coord *first, int sizeFir, struct coord *second, int sizeSec);

int writeClosestToStdout(struct coord c1[2], int ret1, struct coord c2[2], int ret2, struct coord c3[2], int ret3);

#endif