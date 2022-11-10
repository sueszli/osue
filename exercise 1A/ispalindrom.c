/**
 * @file ispalindrom.c
 * @author Yahya Jabary <mailto: yahya.jabary@tuwien.ac.at>
 * @date 09.11.2022
 *
 * @brief Checks if a string read from console or defined file is a palindrome
 * and writes the results back to the console or another defined file.
 *
 * @details This program checks if a string is a palindrome or not. A palindrome
 * is a word, phrase, number, or other sequence of characters which reads the
 * same backward as forward, such as madam or racecar.
 * After checking whether the given string <X> read from the console or a file
 * is a palindrome or not, the program either writes "<X> is a palindrom"
 * [sic] or "<X> is not a palindrom" [sic] to the console or to a file.
 * The program can also be set to ignore white spaces and be case insensitive.
 */

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Macros for formatting error messages and exiting or printing optional
 * debug messages if the flag DDEBUG is set.
 */

// print macros ::
#ifdef DEBUG
#define log(fmt, ...) \
  fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define log(fmt, ...) /* NOP */
#endif

#define error(s)                                   \
  fprintf(stderr, "%s: %s\n", s, strerror(errno)); \
  exit(EXIT_FAILURE);

#define argumentError(s)                                                     \
  fprintf(stderr, "%s\n", s);                                                \
  fprintf(stderr, "%s\n", "USAGE:");                                         \
  fprintf(stderr, "\t%s\n", "ispalindrom [-s] [-i] [-o outfile] [file...]"); \
  fprintf(stderr, "\t%s\n", "s -> trim whitespaces before checking");        \
  fprintf(stderr, "\t%s\n", "i -> ignore letter casing when checking");      \
  fprintf(stderr, "\t%s\n", "o -> output path (default: stdout)");           \
  fprintf(stderr, "\t%s\n", "files -> input paths (default: stdin)");        \
  exit(EXIT_FAILURE);
// :: print macros

// function prototypes ::
/**
 * @brief Parses commandline arguments, sets flags, iterates through input
 * file(s) if necessary, iterates through each "line", passes each line to
 * "writeUpdatedLine()" and then frees the memory allocated for the line.
 * @param argc number of commandline arguments
 * @param argv array of commandline arguments
 * @return EXIT_SUCCESS if program ran successfully, EXIT_FAILURE otherwise
 */
int main(int argc, char **argv);

/**
 * @brief Checks if a string is a palindrome or not and writes either "<X> is a
 * palindrom" [sic] or "<X> is not a palindrom" [sic] to the console or to a
 * file.
 * @param line string to check
 * @param ignoreWhiteSpaces flag to ignore white spaces (treated as boolean)
 * @param ignoreLetterCasing flag to ignore letter casing (treated as boolean)
 * @param outputStream stdout or file to write the initial input + calculated
 * message to
 */
static void writeUpdatedLine(char *line, uint8_t ignoreWhitespaces,
                             uint8_t ignoreLetterCasing, FILE *outputStream);
/**
 * @brief Removes all whitespaces given argument by placing them at the
 * beggining and then placing a null terminator after the last placed
 * non-whitespace. -> This means that the argument will contain 2 null
 * terminators.
 * @param line string to remove whitespaces from
 */
static void trim(char *line);

/**
 * @brief Replaces all uppercase letters with their lowercase equivalent in the
 * given argument.
 * @param line string to replace uppercase letters with lowercase letters
 */
static void toLowerCase(char *line);

/**
 * @brief Checks if a string is a palindrome or not and returns boolean value
 * based on the result.
 * @param line string to check
 */
static _Bool isPalindrome(char *line);

/**
 * @brief Allocates memory and concatenates the two given strings.
 * @param str1 first string which will be placed at the beginning of the new
 * string
 * @param str2 second string which will be placed at the end of the new string
 * @return pointer to the new string (make sure to free it after use)
 */
static char *concat(char *str1, char *str2);
// :: function prototypes

int main(int argc, char **argv) {
  uint8_t ignoreLetterCasing = 0;
  uint8_t ignoreWhitespaces = 0;
  uint8_t writeToFile = 0;
  char *outputPath = NULL;

  int option;
  struct option config[] = {{"outfile", required_argument, 0, 'o'}};
  while (true) {
    int option_index = 0;
    option = getopt_long(argc, argv, "sio:", config, &option_index);

    if (option == -1) {
      break;
    }
    switch (option) {
      case 's':
        log("%s", "option s: ignore whitespaces");
        ignoreWhitespaces++;
        break;
      case 'i':
        log("%s", "option i: ignore letter casing");
        ignoreLetterCasing++;
        break;
      case 'o':
        log("%s", "option o: write output to file");
        writeToFile++;
        outputPath = optarg;
        break;
      default:
        argumentError("Invalid option was used.");
    }
  }
  if (ignoreLetterCasing > 1 || ignoreWhitespaces > 1 || writeToFile > 1) {
    argumentError("The same option was used twice or more often.");
  }

  // open output
  FILE *outputStream;
  if (outputPath != NULL) {
    outputStream = fopen(outputPath, "w");
    if (outputStream == NULL) {
      error("fopen failed");
    }
  } else {
    outputStream = stdout;
  }

  const int numInputFiles = argc - optind;
  log("%s %d", "num of input files:", numInputFiles);
  if (numInputFiles > 0) {
    // iterate through files, send each line to writeUpdatedLine()
    while (argc > optind) {
      char *inputPath = argv[optind++];
      log("%s: %s", "reading content of", inputPath);
      FILE *inputStream = fopen(inputPath, "r");
      if (inputStream == NULL) {
        error("fopen failed");
      }
      char *line = NULL;
      size_t len = 0;
      while (getline(&line, &len, inputStream) != -1) {
        if (strlen(line) == 0) {
          continue;
        }
        writeUpdatedLine(line, ignoreWhitespaces, ignoreLetterCasing,
                         outputStream);
      }
      free(line);
      if (fclose(inputStream) == EOF) {
        error("fclose failed");
      }
    }
  } else {
    // get user input from stdin, send line to writeUpdatedLine()
    char *line = NULL;
    size_t len = 0;
    if (getline(&line, &len, stdin) == -1) {
      error("reading from stdin with getline failed");
    }
    writeUpdatedLine(line, ignoreWhitespaces, ignoreLetterCasing, outputStream);
    free(line);
  }

  // close output
  if (fclose(outputStream) == EOF) {
    error("fclose failed");
  }
  return EXIT_SUCCESS;
}

static void writeUpdatedLine(char *line, uint8_t ignoreWhitespaces,
                             uint8_t ignoreLetterCasing, FILE *outputStream) {
  line[strlen(line) - 1] = '\0';  // remove '\n' at the end

  // get msg
  char *copy = strdup(line);
  if (copy == NULL) {
    error("strdup failed");
  }
  if (ignoreWhitespaces) {
    trim(copy);
  }
  if (ignoreLetterCasing) {
    toLowerCase(copy);
  }
  char *msg =
      isPalindrome(copy) ? " is a palindrom\n" : " is not a palindrom\n";
  free(copy);

  // write output (output = line + msg)
  char *output = concat(line, msg);
  fprintf(outputStream, "%s", output);
  free(output);
}

static void trim(char *line) {
  size_t newLength = 0;
  char *readerp = line;
  char *writerp = line;

  while (*readerp != '\0') {
    if (!isspace(*readerp)) {
      *writerp = *readerp;
      writerp++;
      newLength++;
    }
    readerp++;
  }
  *writerp = '\0';
}

static void toLowerCase(char *line) {
  char *p = line;
  while (*p != '\0') {
    *p = tolower(*p);
    p++;
  }
}

static _Bool isPalindrome(char *line) {
  const size_t length = strlen(line);
  const size_t halfLength = length >> 1;

  if (length == 0) {
    return false;
  }

  for (size_t i = 0; i < halfLength; i++) {
    char left = line[i];
    char right = line[length - (1 + i)];
    if (left != right) {
      return false;
    }
  }
  return true;
}

static char *concat(char *str1, char *str2) {
  size_t size = sizeof(char) * (strlen(str1) + strlen(str2)) + 1;
  char *output = (char *)malloc(size);
  if (output == NULL) {
    error("malloc failed");
  }

  strncpy(output, str1, strlen(str1));
  char *endOfFirstCopy = output + strlen(str1);
  strncpy(endOfFirstCopy, str2, strlen(str2));
  *(endOfFirstCopy + strlen(str2)) = '\0';
  return output;
}
