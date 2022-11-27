/*
*
*   @file cpair.h
*
*   @author Jakob Frank (11837319)
*
*   @brief  defines methods structs and variables to be used by cpair.c
*
*   @date   10/12/2021
*/


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <error.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <float.h>

typedef struct point
{
    double xCoordinate;
    double yCoordinate;
} point, pt_t;

typedef struct point_pair
{
    point p1;
    point p2;
    double dist;
} pair_t;