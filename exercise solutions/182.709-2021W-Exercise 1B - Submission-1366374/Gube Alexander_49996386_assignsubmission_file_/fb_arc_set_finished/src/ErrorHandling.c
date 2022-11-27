/**
 * @file ErrorHandling.c
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief implements error handling function(s)
 *
 * This module contains function which facilitate the handling of errors and notifying
 * the user in case of errors.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * @details no restriction of visibility, the program is terminated with EXIT_FAILURE code in case it has to terminate
 */
void failedWithError(char* progName, char* description, int terminate) {
    fprintf(stderr, "[%s] ERROR - %s: %s\n", progName, description, strerror(errno));
    if(terminate != 1) return;
    exit(EXIT_FAILURE);
}
