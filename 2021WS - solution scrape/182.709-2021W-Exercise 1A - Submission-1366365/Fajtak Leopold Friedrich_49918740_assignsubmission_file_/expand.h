/**
 * @file expand.h
 * @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
 * @brief Utilities for mygrep
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef EXPAND_H
#define EXPAND_H

#include "strutil.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

/**
 * @brief for each line in input, replaces tab characters with the minimal amount of spaces, such that 
 * there is at least one space replacing each tab, and the index of the succeeding character is a multiple
 * of tabstop, and prints it to output
 * 
 * @param input stream to read from
 * @param output stream to print to
 * @param tabstop indent size
 * @return 0 if the function terminates without errors, else 1
 */
int expand(FILE *input, FILE *output, int tabstop);

#endif
