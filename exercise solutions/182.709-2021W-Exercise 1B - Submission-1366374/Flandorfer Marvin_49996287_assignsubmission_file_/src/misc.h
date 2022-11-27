/**
 * Miscellaneous module
 * @file misc.h
 * @author Marvin Flandorfer, 52004069
 * @date 30.10.2021
 * 
 * @brief Module for miscellaneous functions
 * @details This module contains functions for the usage of the programs and error messages.
 */

#ifndef MISC_H
#define MISC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Usage function for generators
 * @brief This function prints the usage of the generator program to stderr.
 * @details Global variables: program_name
 */
void usage_generator(void);

/**
 * Usage function for supervisors
 * @brief This function prints the usage of the supervisor program to stderr.
 * @details Global variables: program_name
 */
void usage_supervisor(void);

/**
 * Error message function
 * @brief This function prints an error message to stderr.
 * @details The error message contains the function name of the erroneous function and the error according to errno
 * @details Global variables: program_name, errno
 * 
 * @param function_name Name of the erroneous function
 */
void error_message(char* function_name);

#endif