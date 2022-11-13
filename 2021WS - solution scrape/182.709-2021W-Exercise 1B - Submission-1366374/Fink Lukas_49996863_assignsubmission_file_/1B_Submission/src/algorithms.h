/**
 * @file algorithms.h
 * @author Lukas Fink 11911069
 * @date 10.11.2021
 *  
 * @brief algorithms Module
 *
 * contains the fisher_yates_shuffle algorithm
 * and an algorithm for generating feedback-Arcs
 **/

#ifndef ALGO_H
#define ALGO_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "datatypes.h"

int * fisher_yates_shuffle(int *vertices);
struct FeedbackArc * fas_algorithm(int *vertices, struct Edge *edges, char *programName);

#endif