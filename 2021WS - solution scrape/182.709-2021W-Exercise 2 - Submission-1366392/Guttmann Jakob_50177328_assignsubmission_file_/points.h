/**
 * @file points.h
 * @author Jakob Guttmann 11810289 <e11810289@student.tuwien.ac.at
 * @brief useful functions to handle with data structure Point
 * @date 04.12.2021
 * 
 * 
 */
#ifndef __POINTS_H__
#define __POINTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>

/**
 * @brief macro for errorhandling, x is the message to be printed and exits with EXIT_FAILURE
 * 
 */
#define ERRORHANDLING(x)                                                                      \
    {                                                                                         \
        fprintf(stderr, "[%s] ERROR: cannot %s\nReason: %s\n", argv[0], #x, strerror(errno)); \
        exit(EXIT_FAILURE);                                                                   \
    }

/**
 * @brief initial size of the array which stores the points
 * 
 */
#define INIT_SIZE_ARRAY 4

/**
 * @brief simple (and trivial) data structure for 2d-points
 * x is position of x-axis
 * y is position of y-axis
 * 
 */
typedef struct points {
    float x;
    float y;
} Point;

/**
 * @brief repositions the points in the array considering the x-value of the points
 * first the method calculates the mean of all x-values
 * then the array is "splitted" in two halves: one with x-value smaller and other x-value greater than the mean
 * special case: x-value equals mean -> then the points are switching between low and higher side
 * Attention: points vector is changed (order of points)
 * 
 * @param points array of points which stores the points from the input, must have size of length (other parameter)
 * order of points is changed, on the lower half there are points with x-values smaller than mean and on the other with x-values higher than mean
 * return value indicates the number of points which are stored on the lower half
 * @param length size of array
 * @return int returns the number of points, which x-value is smaller than the arithmetic mean of all x-values
 */
int rearrangeArray(Point* points, int length);

/**
 * @brief swaps the points at index i and j
 * 
 * @param points points array
 * @param i index i (between 0 and length of array)
 * @param j index j (between 0 and length of array)
 */
void swapEntry(Point* points, int i, int j);

/**
 * @brief prints the points of the array in form "x y" and each point is in one line 
 * 
 * @param points array of points to be printed
 * @param length size of the array
 * @param stream output stream to be printed on
 * @return int returns 0 if all points could be printed, -1 if there occured an error while writing
 */
int printPoints(Point* points, int length, FILE* stream);

/**
 * @brief parses a point from one line, line must be of form "x y"
 * if format is not suitable or point is infinity or NaN method returns -1
 * 
 * @param line line to be parsed to a point
 * @param point values are stored in point
 * @return int returns 0 on success, -1 on error (wrong format or infinity or nan)
 */
int parsePoint(char* line, Point* point);

/**
 * @brief computes the distance between the two given points
 * 
 * @param p1 Point 1
 * @param p2 Point 2
 * @return float distance between the points
 */
float distancePoints(Point p1, Point p2);


#endif