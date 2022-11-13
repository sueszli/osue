/**
 * @file   myexpand.h
 * @author Keisi Cela, 11737582, <e11737582@student.tuwien.ac.at>
 * @date   14.11.2021
 *
 * @brief Provides types and initialization methods used by myexpand.c.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

/* === Global Variables === */

/* Name of the program */
static const char *progName = "myexpand"; /* default name */

/* tabstop value */
char* tabstop = NULL;

/* outfile value */
char* outfile = NULL;

/* Number of files */
int filesNumber = 0;

/* === Prototypes === */

/**
 * @brief prints a usage message and terminate program on program error when input in invalid
 */
static void usage(void);

/**
 * @brief reads given input   
 * @param argc The argument counter
 * @param argv The argument vector
 * @param the list of file names
 */
static char **readInput(int argc, char **argv);

/**
 * @brief replaces tabs with spaces
 * @param file the stream to read
 * @param t the tabstop value
 */
static void replaceTabs(FILE* file,int t, FILE* output);
