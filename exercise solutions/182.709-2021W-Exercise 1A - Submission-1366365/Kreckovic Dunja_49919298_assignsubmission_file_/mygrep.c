/**
 * @file mygrep.c
 * @author Dunja Kreckovic (11928274)
 * @date 7.11.2021
 *
 * @brief Contains the functionality of the reduced variation of the Unix-command grep.
 *
 * @details Program reads in several files and prints all lines containing a keyword. If the option -o is given, the output is written to the specified file (outfile). Otherwise, the output is written to stdout. If the option -i is given, the program shall not differentiate between lower and upper case letters. If one or multiple input files are specified, then the program reads each of them in the order they are given. If no input file is specified, the program reads from stdin.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

enum boolean {
  FALSE = 0, TRUE = 1
};

char * myprog; // program name

/**
 * @brief Prints usage message and terminates the program.
 *
 * @details myprog - Global variable used in this function. It contains the program name specified in argv[0].
 **/
static void usage(void) {
  fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", myprog);
  exit(EXIT_FAILURE);
}

/**
 * @brief Prints error message and terminates the program.
 *
 * @details myprog - Global variable used in this function. It contains the program name specified in argv[0].
 *
 * @param message - Message that should be printed on stderr.
 **/
static void print_error_and_terminate(char * message) {
  fprintf(stderr, message, myprog, strerror(errno));
  exit(EXIT_FAILURE);
}

/**
 * @brief Converts passed string in lowercase.
 *
 * @param str - String that should be coverted in lowercase.
 *
 * @return Returns converted string or NULL in case of error in allocationg mamory.
 **/
static char * get_lowercase(char * str) {
  size_t len = strlen(str);

  char * lowercase = malloc(len * sizeof(char));
  if (lowercase == NULL) { // allocation failed
    return NULL;
  }

  for (size_t i = 0; i < len; ++i) {
    lowercase[i] = tolower(str[i]);
  }
  return lowercase;
}

/**
 * @brief Prints passed line in output determined by output_path only if line contains keyword concerning case sensitivity.
 *
 * @param line - Line that could be outputted.
 * @param csensitive - Parameter that indicates whether the case sensitivity should be concerned or not.
 * @param keyword - Keyword for comparison
 * @param output_path - Determines where the lines will be printed 
 **/
static void put_out(char * line, int c_sensitive, char * keyword, char * output_path) {
  FILE * output;
  if (strcmp(output_path, "stdout") == 0) {
    output = stdout;
  } else {
    output = fopen(output_path, "a");
    if (output == NULL) {
      free(line);
      print_error_and_terminate("[%s] ERROR: fopen failed: %s\n");
    }
  }
  if (c_sensitive == FALSE) {
    char * lowercase_line = get_lowercase(line);
    if (lowercase_line == NULL) {
      free(line);
      print_error_and_terminate("[%s] ERROR: memory allocation failed: %s\n");
    }
    if (strstr(lowercase_line, keyword) != NULL) {
      if (fputs(line, output) == EOF) {
        free(lowercase_line);
        free(line);
        print_error_and_terminate("[%s] ERROR: printing in file failed: %s\n");
      }
    }
    free(lowercase_line);
  } else {
    if (strstr(line, keyword) != NULL) {
      if (fputs(line, output) == EOF) {
        free(line);
        print_error_and_terminate("[%s] ERROR: printing in file failed: %s\n");
      }
    }
  }
  if (strcmp(output_path, "stdout") != 0) {
    if (fclose(output) != 0) {
      free(line);
      print_error_and_terminate("[%s] ERROR: fclose failed: %s\n");
    }
  }
}

/**
 * @brief The starting point of the mygrep program.
 *
 * @details This function performs all the functionalities of this program. It first checks the positional arguments and in case of incorrect program calling terminates. Than it sets all nessesary variables and reads files line by line and for each line check weather it contains the search term. The line is printed only if it contains a keyword. 
 * myprog - Global variable used in this function. It contains the program name specified in argv[0].
 *
 * @param argc - Number of argument values. 
 * @param argv[] - Argument values. 
 *
 * @return Returns EXIT_SUCESS upon success or EXIT_FAILURE upon failure.
 **/
int main(int argc, char * argv[]) {
  myprog = argv[0];

  if (argc < 2) {
    usage();
  }

  // handle options block
  int case_sensitive = TRUE;
  char * output_path = "stdout";
  int c;
  while ((c = getopt(argc, argv, "io:")) != -1) {
    switch (c) {
    case 'i':
      case_sensitive = FALSE;
      break;
    case 'o':
      output_path = optarg;
      break;
    default:
      usage();
    }
  }
  //endblock

  if ((argc - optind) == 0) {
    usage();
  }

  char * keyword = argv[optind++];

  // optimise keyword for comparson if -i is passed
  if (case_sensitive == FALSE) {
    char * new_keyword = get_lowercase(keyword);
    if (new_keyword == NULL) {
      print_error_and_terminate("[%s] ERROR: memory allocation failed: %s\n");
    }
    strcpy(keyword, new_keyword);
    free(new_keyword);
  }

  char * line = NULL;
  size_t len = 0;
  ssize_t lineSize = 0;

  if (argc - optind == 0) { // reads form stdin
    while ((lineSize = getline( & line, & len, stdin)) != -1) {
      put_out(line, case_sensitive, keyword, output_path);
    }
    if (!feof(stdin)) {
      free(line);
      print_error_and_terminate("[%s] ERROR: getline failed: %s\n");
    }
  } else { // reads form files
    while (argc - optind != 0) {
      FILE * file = fopen(argv[optind++], "r");
      if (file == NULL) {
        print_error_and_terminate("[%s] ERROR: fopen failed: %s\n");
      }
      while ((lineSize = getline( & line, & len, file)) != -1) {
        put_out(line, case_sensitive, keyword, output_path);
      }
      if (!feof(file)) {
        free(line);
        print_error_and_terminate("[%s] ERROR: getline failed: %s\n");
      }

      if (fclose(file) != 0) {
        free(line);
        print_error_and_terminate("[%s] ERROR: fclose failed: %s\n");
      }
    }
  }

  free(line);

  return EXIT_SUCCESS;
}