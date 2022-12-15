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

#define errorUsage(msg)                                                  \
  do {                                                                   \
    fprintf(stderr, "%s\n", msg);                                        \
    fprintf(stderr,                                                      \
            "Usage: ./ispalindrome [-s] [-i] [-o outfile] [file...]\n"); \
    exit(EXIT_FAILURE);                                                  \
  } while (0);

static void writeResult(char line[], bool ignoreWhitespace,
                        bool ignoreLetterCasing, FILE *outputStream) {
  line[strlen(line) - 1] = '\0';
  fprintf(outputStream, "%s ", line);

  if (ignoreWhitespace) {
    size_t copyIndex = 0;
    for (size_t i = 0; i < strlen(line); i++) {
      if (!isblank(line[i])) {
        line[copyIndex++] = line[i];
      }
    }
    line[copyIndex] = '\0';
  }

  if (ignoreLetterCasing) {
    for (size_t i = 0; i < strlen(line); i++) {
      line[i] = (char)tolower(line[i]);
    }
  }

  bool isPalindrome = true;
  const size_t len = strlen(line);
  for (size_t i = 0; i < len; i++) {
    if (line[i] != line[len - i - 1]) {
      isPalindrome = false;
    }
  }

  if (isPalindrome) {
    fprintf(outputStream, "is a palindrome\n");
  } else {
    fprintf(outputStream, "is not a palindrome\n");
  }
}

int main(int argc, char *argv[]) {
  bool ignoreWhitespace = false;
  bool ignoreLetterCasing = false;
  bool useOutputPath = false;

  FILE *outputStream = stdout;

  int opt;
  while ((opt = getopt(argc, argv, "sio:")) != -1) {
    switch (opt) {
      case 's':
        if (ignoreWhitespace) {
          errorUsage("repeated use of option");
        }
        ignoreWhitespace = true;
        fprintf(stderr, "-s\n");
        break;

      case 'i':
        if (ignoreLetterCasing) {
          errorUsage("repeated use of option");
        }
        ignoreLetterCasing = true;
        fprintf(stderr, "-i\n");
        break;

      case 'o':
        if (useOutputPath) {
          errorUsage("repeated use of option");
        }
        useOutputPath = true;
        if (optarg[0] == '-') {
          errorUsage("missing argument");
        }
        outputStream = fopen(optarg, "w+");
        if (outputStream == NULL) {
          errorHandler("fopen");
        }
        fprintf(stderr, "-o %s\n", optarg);
        break;

      default:
        errorUsage("invalid option");
    }
  }

  do {
    FILE *inputStream = NULL;
    if ((argc - optind) == 0) {
      inputStream = stdin;
    } else {
      fprintf(stderr, "processing path: %s\n", argv[optind]);
      inputStream = fopen(argv[optind], "r");
      if (inputStream == NULL) {
        errorHandler("fopen");
      }
    }

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, inputStream) != -1) {
      writeResult(line, ignoreWhitespace, ignoreLetterCasing, outputStream);
    }
    free(line);

    if (fclose(inputStream) == EOF) {
      errorHandler("fclose");
    }
  } while (++optind < argc);

  if (fclose(outputStream) == EOF) {
    errorHandler("fclose");
  }

  return EXIT_SUCCESS;
}
