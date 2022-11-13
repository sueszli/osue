/**
 * @file help_functions.h
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 10.12.2021
 *
 * @brief This module provides the type 'point_t' the program 'cpair'.
 *      
 **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

typedef struct point{
    float x_coord;
    float y_coord;
} point_t;