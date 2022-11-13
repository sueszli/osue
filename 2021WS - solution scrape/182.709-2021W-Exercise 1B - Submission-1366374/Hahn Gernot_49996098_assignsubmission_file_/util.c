/**
 * @file util.c
 * @author Gernot Hahn 01304618
 * @date 14.11.2021
 *
 * @brief Implementation of the util module
 *
 * @details This module provides implemented utility functions
 * for programs.
 */

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/**
 * @brief This function implements the error handler.
 * @detail This function writes a error message to stderr.
 * @param prog_name The name of the program, in which the
 * error occurred.
 * @param msg The message wich shall be printed.
 */
void err_handler(const char* prog_name, const char *msg) {
	fprintf(stderr, "[%s] ERROR: %s: %s\n", prog_name, msg, strerror(errno));
}

