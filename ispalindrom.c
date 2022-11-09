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

// region "print macros"
#ifdef DEBUG
#define log(fmt, ...) \
  fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define log(ignore)
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
// endregion

/**
 * input gets reallocated (must be freed)
 */
static void trim(char *line) {
  // https://stackoverflow.com/questions/74350465/how-to-free-memory-following-a-0-placed-somewhere-in-a-string-in-c

  int8_t newLength = 0;
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

  // delete memory following '\0'
  size_t size = sizeof(char) * newLength;
  char *shorterLine = (char *)malloc(size);
  if (shorterLine == NULL) {
    error("malloc failed");
  }
  strncpy(shorterLine, line, newLength);
  line = shorterLine;
  free(line);
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

/**
 * returns new string
 * output = str1 + str2
 */
static char *strConcat(char *str1, char *str2) {
  size_t size = sizeof(char) * (strlen(str1) + strlen(str2)) + 1;
  char *output = (char *)malloc(size);
  if (output == NULL) {
    error("malloc failed");
  }

  strncpy(output, str1, strlen(str1));
  char *endOfCopy = output + strlen(str1);
  strncpy(endOfCopy, str2, strlen(str2));
  *(endOfCopy + strlen(str2)) = '\0';
  return output;
}

static void writeUpdatedLine(char *line, uint8_t ignoreWhitespaces,
                             uint8_t ignoreLetterCasing, FILE *outputStream) {
  line[strlen(line) - 1] = '\0';  // remove '\n' at the end

  // get msg
  char *copy = strdup(line);
  if (ignoreWhitespaces) {
    trim(copy);
  }
  if (ignoreLetterCasing) {
    toLowerCase(copy);
  }
  char *msg =
      isPalindrome(copy) ? " is a palindrom\n" : " is not a palindrom\n";
  free(copy);

  // output = line + msg
  char *output = strConcat(line, msg);

  // write
  fprintf(outputStream, "%s", output);
  free(output);
}

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

  // set output
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
    // iterate through files
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
        writeUpdatedLine(line, ignoreWhitespaces, ignoreLetterCasing,
                         outputStream);
        free(line);
      }
      fclose(inputStream);
    }
  } else {
    // get user input from stdin
    char *line = NULL;  // -> gets free'd in writeUpdatedLine
    size_t len = 0;
    if (getline(&line, &len, stdin) == -1) {
      error("reading from stdin with getline failed");
    }
    writeUpdatedLine(line, ignoreWhitespaces, ignoreLetterCasing, outputStream);
    free(line);
  }

  fclose(outputStream);
  return EXIT_SUCCESS;
}
