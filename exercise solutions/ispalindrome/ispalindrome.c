#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define errorHandler(msg) \
  do {                    \
    perror(msg);          \
    exit(EXIT_FAILURE);   \
  } while (0);

#define errorUsage(msg)                                                 \
  do {                                                                  \
    fprintf(stderr,                                                     \
            "Wrong usage: %s\nSYNOPSIS:\n\tispalindrome [-s] [-i] [-o " \
            "outfile] [file...]\n",                                     \
            msg);                                                       \
    exit(EXIT_FAILURE);                                                 \
  } while (0);

static void removeWhitespace(char line[]) {
  // side effect: line
  // line may contain two '\0' characters
  const size_t len = strlen(line);
  size_t copyIndex = 0;
  for (size_t i = 0; i < len; i++) {
    if (!isblank(line[i])) {
      line[copyIndex++] = line[i];
    }
  }
  line[copyIndex] = '\0';
}

static void toLowerCase(char line[]) {
  // side effect: line
  const size_t len = strlen(line);
  for (size_t i = 0; i < len; i++) {
    line[i] = (char)tolower(line[i]);
  }
}

static bool isPalindrome(char line[]) {
  const size_t len = strlen(line);
  for (size_t i = 0; i < len; i++) {
    if (line[i] != line[len - i - 1]) {
      return false;
    }
  }
  return true;
}

static void writeResult(char line[], bool ignoreWhitespace,
                        bool ignoreLetterCasing, FILE *outputStream) {
  // side effect: line
  // line will contain two '\0' characters
  line[strlen(line) - 1] = '\0';
  fprintf(outputStream, "%s ", line);

  if (ignoreWhitespace) {
    removeWhitespace(line);
  }
  if (ignoreLetterCasing) {
    toLowerCase(line);
  }

  if (isPalindrome(line)) {
    fprintf(outputStream, "is a palindrome\n");
  } else {
    fprintf(outputStream, "is not a palindrome\n");
  }
}

int main(int argc, char *argv[]) {
  bool ignoreWhitespace = false;
  bool ignoreLetterCasing = false;
  FILE *outputStream = stdout;

  int opt;
  while ((opt = getopt(argc, argv, "sio:")) != -1) {
    switch (opt) {
      case 's':
        ignoreWhitespace = true;
        break;

      case 'i':
        ignoreLetterCasing = true;
        break;

      case 'o':
        outputStream = fopen(optarg, "w+");
        if (outputStream == NULL) {
          errorHandler("fopen");
        }
        break;

      default:
        errorUsage("invalid option");
    }
  }

  bool hasInputPaths = optind < argc;
  char *line = NULL;
  size_t len = 0;

  // read from input paths (if there any)
  while (optind < argc) {
    FILE *inputStream = fopen(argv[optind++], "r");
    if (inputStream == NULL) {
      errorHandler("fopen");
    }
    while (getline(&line, &len, inputStream) != -1) {
      writeResult(line, ignoreWhitespace, ignoreLetterCasing, outputStream);
    }
    if (fclose(inputStream) == EOF) {
      errorHandler("fclose");
    }
  }

  // read from stdin (if there are not any)
  if (!hasInputPaths) {
    while (getline(&line, &len, stdin) != -1) {
      writeResult(line, ignoreWhitespace, ignoreLetterCasing, outputStream);
    }
  }

  free(line);

  if (fclose(outputStream) == EOF) {
    errorHandler("fclose");
  }
  return EXIT_SUCCESS;
}
