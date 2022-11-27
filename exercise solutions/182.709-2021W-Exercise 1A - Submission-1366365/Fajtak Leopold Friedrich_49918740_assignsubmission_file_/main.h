/**
 * @file main.h
 * @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
 * @brief The program takes as argument an optional integer -t, an optional
 * optput file -o, as well as multiple input files, where the default values
 * for -t and -o are 8 and stdout, respectively.  For each line in the input
 * files, it replaces each tabstop character with a minimal number of spaces,
 * such that there is at least one space replacing each tabstop, and the
 * character succeeding each tab, is at an index which is a multiple of -t.
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef MAIN_H
#define MAIN_H

#include "fileutil.h"
#include "expand.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/**
 * Program entry point
 * @brief parses arguments and processes that input
 * 
 * @param argc Argument counter
 * @param argv Argument vector
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE on success or failure respectively
 */
int main(int argc, char ** argv);

#endif
