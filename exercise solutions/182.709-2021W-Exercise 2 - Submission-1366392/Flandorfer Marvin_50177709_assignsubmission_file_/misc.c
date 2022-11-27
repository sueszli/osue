/**
 * @file misc.c
 * @author Marvin Flandorfer, 52004069
 * @date 22.11.2021
 * 
 * @brief Implementation of the Miscellaneous module.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

extern char* program_name;

void error_message(char *function){
    (void) fprintf(stderr, "[%s] Error: %s failed: %s\n", program_name, function, strerror(errno));
}

void usage(void){
    (void) fprintf(stderr, "[%s] Usage: cpair\n", program_name);
}