#ifndef UTILS_H
#define UTILS_H

/**
 * 
 * @description: utils contains all functions used in assA.c to make the code look cleaner
 * 
 * @param BUFF_SIZE: this is the prespecified maximum length of a string read by fgets
 * @param evaluate: evaluate checks whether the line is a palindrom. It takes the source
 *                  and flags, and based on the flags checks whether the line satisfied the
 *                  specifications. It then returns the source line and the verdict as one
 *                  string.
 * @param appendToFullResult: this function takes a string with allocated memory and appends
 *                            a string to it by reallocating the original string and merging
 *                            the two.
 * @param fileManager: the file manager takes all source files from the console input, opens
 *                     each of them and passes them line-by-line to evaluate, then appends the
 *                     returned strings to the final output string.
 * 
 * @first_created: 12/11/2021
 * 
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFF_SIZE 4096

char *evaluate(char *source, bool sflag, bool iflag);
void appendToFullResult(char **fullResult, char *whatToAppend);
void fileManager(char **finalOutput, char *fileNames[], int fileNamesStart, int fileNamesCount, bool sflag, bool iflag);

#endif //UTILS_H