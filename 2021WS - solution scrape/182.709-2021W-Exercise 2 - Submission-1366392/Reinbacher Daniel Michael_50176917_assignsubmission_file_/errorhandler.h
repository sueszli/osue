/**
 * @file errorhandler.h
 * @author Daniel Reinbacher (01614435)
 * @date 2021-10-30
 *  
 * @brief Provides functions for error handling.
 *
 * @details This module provides two functions. One explicitly 
 * for usage errors and the other for all other errors.
 */

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

/**
 * @brief the manatory usage function:
 * @details this function is called when the program is used incorrectly. It prints the correct synopsis and
 * exits the program with status EXIT_FAILURE.
 * 
 */
void usage(void);

/**
 * @brief a general error handling function.
 * @details this function is called when a non-usage error occurs. Its prints out useful
 * information about the error and exits the program with status EXIT_FAILURE.
 * 
 * @param title the title of the error as provided by the user
 * @param message the error message
 */
void error_exit(char *title, char *message);

#endif