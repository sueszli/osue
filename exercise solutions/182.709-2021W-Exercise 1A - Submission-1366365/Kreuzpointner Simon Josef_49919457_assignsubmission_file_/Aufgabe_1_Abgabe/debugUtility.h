/**
 * @file debugUtility.h
 * @author Simon Josef Kreuzpointner 12021358
 * @date 27 October 2021
 *
 * @brief the debug utility module
 * 
 * @details This module provides a debug print macro aswell 
 * as the usage function.
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
 * @brief Displays the synopsis
 *
 * @details This function displays the synopsios of the program and
 * will exit afterwards with an exit code of EXIT_FAILURE if exitAfterwards is 1.
 * This function also uses the global variable programName.
 * @see programName
 * 
 * @param exitAfterwards If set to 1 the program will exit after displaying the synopsis, otherwise return void.
 */
void usage(int exitAfterwards);

#endif