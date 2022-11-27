/**
 * @file debugUtility.h
 * @author Simon Josef Kreuzpointner 12021358
 * @date 21 November 2021
 *
 * @brief the debug utility module
 * 
 * @details This module provides a debug print macro aswell as
 * an error, warning and info message functions.
 */

#ifndef DEBUGUTILITY_H
#define DEBUGUTILITY_H

#include <stdio.h>

/**
 * @def debug(...)
 * 
 * @brief Prints the given formatted string to stderr.
 * 
 * @details This macro prints the given formatted string to stderr including the filename aswell as the line number as a prefix.
 * The flag -DDEBUG must be set in the compiler definitions to enable this feature.
 * If not set, nothing will be printed.
 */
#ifdef DEBUG
#define debug(...)                                                       \
    do                                                                   \
    {                                                                    \
        (void)fprintf(stderr, "[%s:%d] [ DEBUG ] ", __FILE__, __LINE__); \
        (void)fprintf(stderr, ##__VA_ARGS__);                            \
        (void)fprintf(stderr, "\n");                                     \
    } while (0)
#else
#define debug(...)
#endif

/**
 * @brief prints an error message
 * 
 * @details This function prints the specified error message to stderr
 * with the program name and given function name as a prefix followed 
 * by the string version of the error number, supplied by errno, in the
 * next line.
 * 
 * The flag -DERROR must be set in the compiler definitions to enable this feature.
 * If not set, nothing will be printed.
 * 
 * The funcion name should be the name only without any braces.
 * @see programName
 * 
 * @param functionName name of the function where the error was thrown
 * @param msg error message
 */
void printError(char *functionName, char *msg);

/**
 * @brief prints a warning message
 * 
 * @details This function prints the specified warning message to stderr
 * with the program name and given function name as a prefix.
 * 
 * The flag -DWARNING must be set in the compiler definitions to enable this feature.
 * If not set, nothing will be printed.
 * 
 * The funcion name should be the name only without any braces.
 * @see programName
 * 
 * @param functionName name of the function where the warning was noted
 * @param msg warning message
 */
void printWarning(char *functionName, char *msg);

/**
 * @brief prints an info message
 * 
 * @details This function prints the specified info message to stderr
 * with the program name and given function name as a prefix.
 * 
 * The flag -DINFO must be set in the compiler definitions to enable this feature.
 * If not set, nothing will be printed.
 * 
 * The funcion name should be the name only without any braces.
 * @see programName
 * 
 * @param functionName name of the function where the info was noted
 * @param msg info message
 */
void printInfo(char *functionName, char *msg);

/**
 * @brief prints the given message with the program name as prefix
 * 
 * @details This function prints the specified message to stdout
 * with the program name as a prefix.
 * 
 * @see programName
 * 
 * @param msg the message
 */
void printName(char *msg);

/**
 * @brief prints the given formatted message with the program name
 * as a prefix
 * 
 * @details This function calls printName() internally with the given
 * formatted string.
 * 
 * @param format string format
 * @param ... format arguments
 * @return Returns 0 on success and -1 otherwise.
 */
int printfName(const char *format, ...);

#endif