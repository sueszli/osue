/**
 * @file supervisor.h
 * @author Leopold Fajtak (e0152033@student.tuwien.ac.at)
 * @brief The supervisor process initializes a shared memory to which generator processes can write solutions of Feedback Arc Set problems
 * Supervisor reads those solutions, compares them in terms of size, saves and prints the smallest one until terminated.
 * Upon a termination signal, the process frees all allocated memory, unlinks shared memory, and tells the supervisor processes to terminate.
 * @version 0.1
 * @date 2020-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "circularbuffer.h"
#include "graph.h"

#include <assert.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

/**
 * @brief Program entry point
 * 
 * @return int 
 */
int main(int argc, char **argv);

/**
 * @brief Signal handler for termination signal. Frees all allocated memory and unliks shared memory objects
 * 
 * @param signal 
 */
void handle_sigterm(int signal);
