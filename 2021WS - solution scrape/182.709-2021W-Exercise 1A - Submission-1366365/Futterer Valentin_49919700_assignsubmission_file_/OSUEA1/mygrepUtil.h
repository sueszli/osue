/**
 * @file mygrepUtil.h
 * @author Valentin Futterer 11904654
 * @date 01.11.2021
 * @brief Provides methods for mygrep.c to use.
 * @details The error and usage function are provided by this file. It also provides the core functionality for mygrep.c.
 * The mygrep method reads lines from the input and uses search_str to search the line for a keyword. If one is found,
 * it is printed to the output file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

/**
 * @brief Exits with error.
 * @details Prints a message to stderr and prints the error code from errno, exits with error.
 * @param prog_name Programme name to print in error message.
 * @param msg The error message to print.
 * @return Returns nothing.
*/
void handle_error(const char *prog_name, char *msg);

/**
 * @brief Displays usage message.
 * @details Prints a message to stderr and shows the correct way to parse arguments to the program, exits with error.
 * @param prog_name Programme name to print in error message.
 * @return Returns nothing.
*/
void usage(const char *prog_name);

/**
 * @brief Reads lines and calls search_str for each line.
 * @details Reads lines from input using getline(). Calls search_str for each line.
 * @param output The File to write the result to.
 * @param input The File to read the lines from.
 * @param case_insensitive 1 if the search should be case insensitive.
 * @param keyword The keyword to be searched for in the read lines.
 * @return Returns a pointer to the first occurence of needle in haystack, if the needle is not found NULL.
 */
char* mygrep(FILE *output, FILE *input, int case_insensitive, char *keyword);