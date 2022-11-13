/**
 * @author Daniel Reinbacher (01614435)
 * @date 2021-10-30
 * @file main.c
 *
 * @brief Main program module. A basic implementation of the grep command.
 *
 * @details This program writes every sentence containing a keyword from either a list of files or stdin 
 * to either an output file or stdout. The program has the following synospis mygrep '[-i] [-o outfile] keyword [file...]'.
 * -o specifies the output file. If the option is not given stdout is used as output. -i if given as an option the program 
 * ignores case sensitivity when searching for the keyword. The first positional argument is the given keyword and every 
 * following argument is an input file from which should be read. If no input files are specified the program reads from stdin. 
 **/

#include "errorhandler.h"
#include "mygrep.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *myprog;

/**
 * Program entry point.
 * @brief The program starts here. First the parameters are checked and afterwards the function 
 * grep is called with the specified input and output stream together with the provided keyword 
 * and an option for case insensitivity when given.
 * @details First the global variable myprog is set to the name of the program. Then the pointers
 * *in and *out are set to stdin and stdout in case no input or output files are specified. Then 
 * the given parameters are checked. If no first positional argument (keyword) has been provided, 
 * the usage() function is called. Afterwards the program checks wheter an output file was specified 
 * and if so sets the pointer *out to that file. Then the program checks if additional positional
 * arguments are given and if not, the function mygrep is called. If there are additional arguments,
 * the program iterates over the arguments, sets the *in pointer to the current file, calls the mygrep
 * function and closes the file.
 * global variables: myprog
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS.
 */
int main(int argc, char *argv[]) {
  myprog = argv[0];
  FILE *in = stdin, *out = stdout;
  char *keyword;
  int opt_i = 0;
  char *o_arg = NULL;
  int c;
  while ((c = getopt(argc, argv, "io:")) != -1) {
    switch (c) {
    case 'i':
      opt_i++;
      break;
    case 'o':
      o_arg = optarg;
      break;
    case '?':
      usage();
      break;
    }
  }
  if ((argc - optind) < 1) {
    usage();
  }
  keyword = argv[optind];
  if (o_arg != NULL) {
    if ((out = fopen(o_arg, "w")) == NULL) {
      error_exit("fopen failed", strerror(errno));
    }
  }
  if ((argc - optind) < 2) {
    mygrep(in, out, keyword, opt_i);
  }
  while (++optind < argc) {
    if ((in = fopen(argv[optind], "r")) == NULL) {
      error_exit("fopen failed", strerror(errno));
    }
    mygrep(in, out, keyword, opt_i);
    fclose(in);
  }
  fclose(out);
  return EXIT_SUCCESS;
}