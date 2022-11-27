/**
 * @file errorhandler.c
 * @author Daniel Reinbacher (01614435)
 * @brief Implementation of the errorhandling module.
 * @details The Implementation provides two functions. One is a usage function
 * called when there is an error is the use of the program. The other one is a
 * function for general error handlingand takes a title and message as input. Both
 * functions exit the program with the status EXIT_FAILURE.
 * global variables: char *myprog is a pointer to the name of the program.
 * @date 2021-10-30
 */

#include <stdio.h>
#include <stdlib.h>
#include "errorhandler.h"

extern char *myprog;

void usage(void) {
  fprintf(stderr, "Usage: %s\n", myprog);
  exit(EXIT_FAILURE);
}

void error_exit(char *title, char *message) {
  fprintf(stderr, "[%s] ERROR: %s: %s\n", myprog, title, message);
  exit(EXIT_FAILURE);
}