/**
 * Miscellaneous modul
 * @file misc.h
 * @author Marvin Flandorfer, 52004069
 * @date 28.10.2021
 * 
 * @brief Miscellaneous modul.
 * Functions that are useful for the program.
 * 
 * @details This module covers functions that are useful in other parts of the programm and do not fit anywhere else.
 * Spefically this covers string concatination, usage error handling and the output of error messages.
 * */

#ifndef MISC_H
#define MISC_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/**
 * Concatination function
 * @brief Concats two strings and allocates necessary memory.
 * @details The concatination uses different string functions and allocates memory for the newly created string.
 * The order of concatination is first s1 and then s2.
 * The allocated memory needs to be freed after usage using the returned pointer.
 * If both parsed pointers are pointing to null, null will be returned.
 * If one of the two pointers is pointing to null, the other string will be returned.
 * It should be noted that in this case memory will still be allocated and a new pointer will be returned and not the original one.
 * All possible errors will be handled within the function and error messages will be written accordingly.
 * 
 * @param s1 First char pointer pointing to a string that will get concatinated.
 * @param s2 Second char pointer pointing to a string that will be appended to the first string.
 * @return On success a char pointer to the concatinated string will be returned. On failure (due to an error) a null-pointer will be returned.
 * */
char *concat(char *s1, char *s2);

/**
 * Usage error function
 * @brief Prints the usage of the program to stderr and exits the program.
 * @details This function should only be called directly from main to ensure that all program exits are handled in main.
 * It should be noted that errors that might occur in fprintf will not get catched, as this function should only be called if an error already occured.
 * @details global variable: program_name
 * */
void usage(void);

/**
 * Error message function
 * @brief Prints an error message to stderr.
 * @details This function prints the error message with the parsed string acting as the name of the erroneous function.
 * The globally set errno will be translated to a string and will be appended to the erro message.
 * It should be noted that errors that might occur in fprintf will not get catched, as this function should only be called if an error already occured.
 * @details global variables: program_name
 * 
 * @param str String that should contain the name of the erroneous function.
 * */
void error_message(char *str);

#endif