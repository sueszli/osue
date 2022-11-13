/**
 * @file cpair.h
 * @author Raphael Pruckner <e11806918@student.tuwien.ac.at>
 * @date 09.12.21
 * @brief Provides structs for cpair
 */
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#pragma once

/**
 * Point type
 * @brief This type represents a 2D-point and has an x- and a y-coordinate
 */ 
typedef struct point
{
    float x;
    float y;
} point_t;

/**
 * Points array type
 * @brief This type represents an array of points and has a pointer to 
 * all points as well as a number of points contained in the array
 */
typedef struct points_array
{
    point_t *points;
    int point_c;
} points_array_t;

/**
 * Pair type
 * @brief This type represents a pair of points.
 */
typedef struct pair
{
    point_t point1;
    point_t point2;
} pair_t;
