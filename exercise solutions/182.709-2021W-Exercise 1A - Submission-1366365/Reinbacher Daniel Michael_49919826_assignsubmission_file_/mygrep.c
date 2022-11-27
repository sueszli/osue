/**
 * @file mygrep.c
 * @author Daniel Reinbacher (01614435)
 * @brief Implamantation of the mygrep module.
 * @date 2021-10-30
 * @details The module contains two functions: to_lower and mygrep. The mygrep function
 * searches for a keyword in a given input and writes every line containing the keyword
 * to the output file. If the param opt_i is greater then 0 case sensitivity is ignored. 
 * The function to_lower is static and only called by the mygrep function. If converts a
 * given string to lowercase.
 * 
 */

#include "mygrep.h"
#include "errorhandler.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern char *keyword;
extern int opt_i;
extern FILE *in, *out;

/**
 * @brief converts a given string to lowercase
 * @details converts each char in the string to lowercase
 * 
 * @param str given string
 */
static void to_lower(char *str) {
  for (int i = 0; str[i]; i++) {
    str[i] = tolower(str[i]);
  }
}

void mygrep(FILE *in, FILE *out, char *keyword, int opt_i) {
  char *line = NULL;
  char *linecpy = NULL;
  size_t len = 0;
  ssize_t nread;
  if (opt_i > 0) {
      to_lower(keyword);
    }
  while ((nread = getline(&line, &len, in)) != -1) {
    linecpy = strdup(line);
    if (opt_i > 0) {
      to_lower(linecpy);
    }
    char *ret = strstr(linecpy, keyword);
    if (ret != NULL) {
      if (fputs(line, out) == EOF) {
        error_exit("fputs failed", strerror(errno));
      }
    }
  }
  if (ferror(in)) {
    error_exit("getline failed", strerror(errno));
  }
  free(line);
  free(linecpy);
}