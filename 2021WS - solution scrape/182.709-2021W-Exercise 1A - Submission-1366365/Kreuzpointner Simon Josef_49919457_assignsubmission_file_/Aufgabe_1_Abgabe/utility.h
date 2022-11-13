/**
 * @file utility.h
 * @author Simon Josef Kreuzpointner 12021358
 * @date 27 October 2021
 *
 * @brief the utility module
 * 
 * @details This module provides general purpose functions that do not fit
 * into other categories.
 */

#ifndef UTILITY_H
#define UTILITY_H

/**
 * @brief returns the number of digits from a given integer
 * 
 * @details This function returns the number of digits of an integer. If the number
 * is negative, the sign is ignored.
 * 
 * @param n an arbitrary integer
 * @return returns the number of digits from the given integer.
 */
int numberOfDigits(int n);

/**
 * @brief appends a string to an existing string
 * 
 * @details This function dynamically allocates and reallocates memory in order to append
 * a given string to another string.
 * If an error ocurres within this operation the program outputs an error message via
 * printError() and returns -1.
 * 
 * @param s this is the main string where content should be appended to
 * @param size the size of s in order to fit content. It will allocate and reallocate memory according to this
 * @param offset the offset to the last element of s. The given string will be appended there
 * @param content the string to be appended
 * @return Returns 0 upon success and -1 on failure.
 */
int appendToString(char **s, size_t size, int offset, char *content);

/**
 * @brief prints n empty lines into the given file stream.
 * 
 * @details This function prints n vertical lines into the given file stream.
 * If an error ocurres within this operation the program outputs an error message via
 * printError() and returns -1.
 * 
 * @param n The amount of empty lines to be printed.
 * @param stream The file stream, where it should be printed.
 * @return Returns 0 upon success and -1 on failure.
 */
int printVerticalSpacing(int n, FILE *stream);

/**
 * @brief prints an error message
 * 
 * @details This function prints the specified error message to stderr
 * followed by the error number suplied by errno.
 * This function also uses the global variable programName.
 * @see programName
 * 
 * @param functionName Name of the function where the error was thrown.
 * @param msg Errormessage
 */
void printError(char *functionName, char *msg);

#endif