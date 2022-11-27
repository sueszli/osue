/**
 * @file generator.c
 * @author Leopold Fajtak (e0152033@student.tuwien.ac.at)
 * @brief The generator scans a given graph and randomly looks for a small amout of edges to remove
 * in order to make it 3-colorable. It writes those soultions to a circularBuffer read by the supervisor
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "circularbuffer.h"
#include "graph.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Program entry point
 * 
 * @param argc Argument counter
 * @param argv Argument vector. All positional arguments have to conform to the pattern %d-%d
 * @return int 
 */
int main(int argc, char** argv);
