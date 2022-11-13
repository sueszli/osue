/**
 * @file util.h
 * @author Gernot Hahn 01304618
 * @date 14.11.2021
 *
 * @brief Provides utility functions useful for programs.
 *
 * @details The util module. Next to a error handler function,
 * this module provides several MAKROS, useful for shared memory,
 * as well as a definition of a shared memory data structure.
 */

#include <signal.h>
#include "graph.h"

#define SHM_NAME "/01304618myshm"
#define SEM_1 "/01304618sem_1"
#define SEM_2 "/01304618sem_2"
#define SEM_3 "/01304618sem_3"
#define CBUF_SIZE (60)

#ifndef UTIL_H
#define UTIL_H

typedef struct myshm {
	fb_arc_set cbuf[CBUF_SIZE];
	volatile sig_atomic_t quit;
	int wr_pos;
} myshm_t;

#endif

/**
 * @brief This function handles errors.
 * @detail This function writes a error message to stderr.
 * @param prog_name The name of the program, in which the
 * error occurred.
 * @param msg The message wich shall be printed.
 */
void err_handler(const char* prog_name, const char *msg);

