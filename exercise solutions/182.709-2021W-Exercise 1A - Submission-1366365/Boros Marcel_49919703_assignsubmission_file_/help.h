/**
 * @file help.h
 * @author Marcel Boros <e11937377@student.tuwien.ac.at>
 * @date 3.11.2021
 *
 * 
 * @brief This program checks whether strings are palindroms or not. There are two options
 * "-s" (ignoring whitespaces) and "-i" (case insensitive) which can be specified during 
 * the check. There is also the possibility, to read from the command line and write 
 * your output into a file, which can be specified with the option "-o". 
 **/

#ifndef HELP_H  //prevent multiple inclusion
#define HELP_H

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define BUFFER_MAX 100      //Random value. Determines how much bytes should be read at once


/**
 * @brief Removes whitespaces from strings
 */
char* removeWhitespaces(char* str) ;


/**
 * @brief Sets a string to lowercase
 */
char* lowerCase(char* str);


/**
 * @brief Separates a line into tokens by the newline-character as a delimiter ("\n"). Furthermore it prints
 * the result of a palindrom-check into a file or to stdout. 
 */
void separateLines(char* prog, char* line_buffer, int opt_s, int opt_i, int opt_o, char* o_arg);


/**
 * @brief Returns the reversed string. Options can be specified in order to manipulate the input string
 * a certain way.
 */
char reverse(char* str, int opt_s, int opt_i);


#endif