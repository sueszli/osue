/**
 * @file mygrep.h
 * @author Daniel Reinbacher (01614435)
 * @brief The mygrep module. This module implements a basic version of the grep command.
 * @details The module contains the function 'mygrep'. Given an input file, output file,
 * keyword and an int >= 0 it reads every line from the input file and copies the lines
 * to the output file that contain the given keyword. If opt_i is greater than 0 case sensitivty
 * is ignored.
 * @date 2021-10-30
 */

#ifndef MYGREP_H
#define MYGREP_H

#include <stdio.h>

/**
 * @brief Implementation of the grep algorithm.
 * @details The functions reads every line from the given input and
 * checks whether its contains the given keyword. If opt_i is greater
 * than 0 it first calls the function to_lower to convert both the current
 * line and the keyword to lowercase letters for case insensitivety. If the 
 * line contains the keyword, the function writes the line to the specified 
 * output. To preserve the original line when converting the input to lowercase
 * the original line is copied beforehand.
 * 
 * @param in given input file
 * @param out given output file
 * @param keyword given keyword
 * @param opt_i > 0 if case insensitive
 */
void mygrep(FILE *in, FILE *out, char *keyword, int opt_i);

#endif