/**
*@file mygrep.h
*@author Jasmine Haider <e11943664@student.tuwien.ac.at>
*@date 14.11.2021
*
*@brief implements a reduced version of grep for a file.
*
* The mygrep module. Checks if a keyword is contained in lines of a file or stdin.
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

/**
*@brief reads lines from a file or stdin and prints the lines containing a keyword to an output file or stdout.
* @details
*@param in The input file to be searched.
*@param out The output file, lines containing the keyword will be printed to this file.
*@param insensitive If this parameter is 0, the search will be case sensitive, if it is 1 the search will not be case sensitive.
@param character pointer to the keyword that should be searched for in the lines.
**/
void mygrep (FILE * in, FILE * out, int insensitive, char *keyword);

/** program name.
 * @brief this name will be used for error messages.
 **/
extern char *prog_name;
