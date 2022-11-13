/**
 * @file main.c
 * @author Jannis Adamek (11809490)
 * @date 2021-11-13
 *
 * @brief Main program module of assignment 1A.
 *
 * Assignment 1A: Simple diffing program that compares two files, similar to
 * diff. SYNOPSIS: mydiff [-i] [-o outfile] file1 file2
 **/

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** The file the program prints to, this is either stdout or a file (-o) */
static FILE *output_file = NULL;
/** first file that gets diffed */
static FILE *left_file = NULL;
/** second file that gets diffed */
static FILE *right_file = NULL;
/** A pointer to the function used for the comparison, this will be different
 * with and without the -i option. */
static int (*str_cmp_func)(const char *, const char *, size_t) = NULL;

/**
 * Display the usage string and close the program
 * @brief Displays the synopsis on stderr and exits the program with
 * EXIT_FAILURE.
 * @param program_name The name of the program (argv[0]).
 */
static void display_usage_and_exit(char *program_name) {
  fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2\n", program_name);
  exit(EXIT_FAILURE);
}

/**
 * Close any open file pointers.
 * @brief Closes output_file, left_file and right_file, if they are open.
 * @details This function must be called before the program exists, if any files
 * have been opened.
 */
static void close_global_files(void) {
  if (output_file != NULL && output_file != stdout) {
    fclose(output_file);
  }
  if (left_file != NULL) {
    fclose(left_file);
  }
  if (right_file != NULL) {
    fclose(right_file);
  }
}

/**
 * Opens a file
 * @brief A wrapper for fopen, if any error occurs all global files get closed
 * and the program terminates with an error message.
 * @param file_name The name (or path) of a file in the file system.
 * @param access The access modifier used by fopen, e.g "r" or "w".
 * @param program_name The name of the running program.
 */
static FILE *open_file(char *file_name, char *access, char *program_name) {
  FILE *file = fopen(file_name, access);
  if (file == NULL) {
    fprintf(stderr, "[%s] ERROR while opening file %s: %s\n", program_name,
            file_name, strerror(errno));
    close_global_files();
    exit(EXIT_FAILURE);
  }
  return file;
}

/**
 * Parse program arguments
 * @brief Checks the correct use of the program options and the two positional
 * arguments. If the program received correct arguments, the function sets the
 * global file pointers as well as the comparison function accordingly.
 * @param argc The argument counter from main.
 * @param argv The argument vector from main.
 */
static void parse_args(int argc, char *argv[]) {
  bool opt_lowercase = false;
  char *output_file_name = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "io:")) != -1) {
    switch (opt) {
    case 'i':
      if (opt_lowercase) {
        display_usage_and_exit(argv[0]);
      }
      opt_lowercase = true;
      break;
    case 'o':
      if (output_file_name != NULL) {
        display_usage_and_exit(argv[0]);
      }
      output_file_name = optarg;
      break;
    case '?':
      display_usage_and_exit(argv[0]);
      break;
    default:
      assert(0);
    }
  }

  // After checking all options there should exactly be two argumetens left, the
  // two files.
  int num_positional_args = argc - optind;
  if (num_positional_args != 2) {
    display_usage_and_exit(argv[0]);
  }

  if (opt_lowercase) {
    str_cmp_func = strncasecmp;
  } else {
    str_cmp_func = strncmp;
  }
  if (output_file_name == NULL) {
    output_file = stdout;
  } else {
    output_file = open_file(output_file_name, "w", argv[0]);
  }
  left_file = open_file(argv[optind], "r", argv[0]);
  right_file = open_file(argv[optind + 1], "r", argv[0]);
}

/**
 * Check if a string is the end of a line.
 * @brief check if a character pointer points to either '\0' or '\n'.
 * @param string The string to be checked.
 * @return true if string points to the end of a line else false
 */
static bool is_line_at_end(char *string) {
  return *string == '\n' || *string == '\0';
}

/**
 * Count the differences between two strings.
 * @brief Count the number of times left_line and right_line have different
 * characters at the same index. Checks only up to the shorter line, e.g "abc\n"
 * and "abcdef\n" have a difference of 0.
 * @details The function uses the global str_cmp_func function to make the
 * actual comparsion, which may or may be case insensitive. (Set by the -i
 * option).
 * @param left_line string from the left file.
 * @param right_line string from the right file.
 * @return The number of times left_line and right_line differ.
 */
static int diff_lines(char *left_line, char *right_line) {
  int diff_count = 0;
  while (!is_line_at_end(left_line) && !is_line_at_end(right_line)) {
    if (str_cmp_func(left_line, right_line, 1) != 0) {
      diff_count++;
    }
    left_line++;
    right_line++;
  }
  return diff_count;
}

/**
 * Diff left_file and right_file
 * @brief Reads left_file and right_file line by line in a loop and prints (to
 * output_file) if diff_lines reports a difference.
 * @details This functions is the only function in this short program that
 * allocates memory on the heap by using getline.
 */
static void diff_global_files(void) {
  size_t left_size = 0;
  size_t right_size = 0;
  char *left_line = NULL;
  char *right_line = NULL;

  int line_number = 1;
  while (getline(&left_line, &left_size, left_file) != -1 &&
         getline(&right_line, &right_size, right_file) != -1) {
    int diff_count = diff_lines(left_line, right_line);
    if (diff_count != 0) {
      fprintf(output_file, "Line: %d, characters: %d\n", line_number,
              diff_count);
    }
    line_number++;
  }
  free(left_line);
  free(right_line);
}

/**
 * Program entry point.
 * @brief parse_args sets the four global variables, the
 * sets the global file pointers and opens them.
 * diff_global_files does the actual diffing and prints the
 * result.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS if parse_args didn't exit with EXIT_FAILURE.
 */
int main(int argc, char *argv[]) {
  parse_args(argc, argv);
  diff_global_files();
  close_global_files();
  return EXIT_SUCCESS;
}
