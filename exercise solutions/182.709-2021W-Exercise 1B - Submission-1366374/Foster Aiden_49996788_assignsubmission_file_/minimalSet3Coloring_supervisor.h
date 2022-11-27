/**
 * @file minimalSet3Coloring_supervisor.h
 * @author Aiden Foster 11910604
 * @date 12.11.2021
 *
 * @brief Supervisor compares the solutions for Minimal Set 3 Coloring provided by generators and finds their minimum
**/

#ifndef MINIMAL_SET_3_COLORING_SUPERVISOR_H
#define MINIMAL_SET_3_COLORING_SUPERVISOR_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include "minimalSet3Coloring.h"

/**
 * @brief Do not realloc array that is shorter than this
**/
#define MIN_REALLOC_SIZE 10

#endif
