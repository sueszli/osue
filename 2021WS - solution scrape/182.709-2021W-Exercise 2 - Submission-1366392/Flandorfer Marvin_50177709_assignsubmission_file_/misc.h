/**
 * @file misc.h
 * @author Marvin Flandorfer, 52004069
 * @date 23.11.2021
 * 
 * @brief Miscellaneous module
 * @details In this module all functions are covered that are useful for other files.
 * Specifically, functions for error handling and usage are here.
 */

#ifndef MISC_H
#define MISC_H

/**
 * Error message function
 * @brief Prints an error message to stderr.
 * @details This function prints the error message with the parsed string acting as the name of the erroneous function.
 * The globally set errno will be translated to a string and will be appended to the erro message.
 * It should be noted that errors that might occur in fprintf will not get catched, as this function should only be called if an error already occured.
 * @details global variables: program_name
 * 
 * @param function String that should contain the name of the erroneous function.
 * */
void error_message(char *function);

/**
 * Usage error function
 * @brief Prints the usage of the program to stderr.
 * @details It should be noted that errors that might occur in fprintf will not get catched, as this function should only be called if an error already occured.
 * @details global variable: program_name
 * */
void usage(void);

#endif