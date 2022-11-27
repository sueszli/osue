/**
 * Miscellaneous module
 * @file misc.c
 * @author Marvin Flandorfer, 52004069
 * @date 30.10.2021
 * 
 * @brief Implementation of miscellaneous module.
 */

#include "misc.h"
#include <errno.h>

extern char* program_name;              /**< Program name*/

void usage_generator(void){
    (void) fprintf(stderr, "[%s] Usage: generator EDGE1...\n", program_name);
}

void usage_supervisor(void){
    (void) fprintf(stderr, "[%s] Usage: supervisor\n", program_name);
}

void error_message(char* function_name){
    (void) fprintf(stderr, "[%s] Error: %s failed: %s\n", program_name, function_name, strerror(errno));
}