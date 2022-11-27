/**
 * @file grep.h
 * @author Marvin Michlits 11731205
 * @date 04.11.2021
 *
 * @brief provides functions used in main.c
 * 
 * grep.h declares some functions which are used in main.c to implement a variation of grep.
 * The declared functions include a function to open an output file, one to write to that output file and one to write to stdout.
 * The implementations of these functions are in grep.c.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

FILE *openOutput(char *filename, char *option);

FILE *writeToOutput(char *originalLine, char *line, char *grepString, FILE *output);

int writeToStd(char *originalLine, char *line, char *grepString);