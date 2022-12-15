#define _GNU_SOURCE
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define usage(msg)                                                           \
  do {                                                                       \
    fprintf(stderr, "%s\n", msg);                                            \
    fprintf(stderr,                                                          \
            "Usage: ./mygrep mygrep [-i] [-o outfile] keyword [file...]\n"); \
    exit(EXIT_FAILURE);                                                      \
  } while (0)

int main(int argc, char* argv[]) {
  bool ignoreLetterCasing = false;

  bool setOutputPath = false;
  FILE* outputStream = stdout;

  int opt;
  while ((opt = getopt(argc, argv, "io:")) != -1) {
    switch (opt) {
      case 'i':
        if (ignoreLetterCasing) {
          usage("multiple usage of option");
        }
        ignoreLetterCasing = true;
        fprintf(stderr, "-i\n");
        break;

      case 'o':
        if (setOutputPath) {
          usage("multiple usage of option");
        }
        setOutputPath = true;
        if ((optarg == NULL) || (optarg[0] == '-')) {
          usage("missing argument");
        }
        outputStream = fopen(optarg, "w+");
        if (outputStream == NULL) {
          error("fopen");
        }
        fprintf(stderr, "-o %s\n", optarg);
        break;

      default:
        usage("unknown option");
    }
  }

  if ((argc - optind) == 0) {
    usage("missing keyword");
  }
  char* keyword = argv[optind++];

  do {
    FILE* inputStream = NULL;
    if ((argc - optind) == 0) {
      inputStream = stdin;
    } else {
      fprintf(stderr, "processing path: %s\n", argv[optind]);
      inputStream = fopen(argv[optind], "r");
      if (inputStream == NULL) {
        error("fopen");
      }
    }

    char* line = NULL;
    size_t len = 0;
    while (getline(&line, &len, inputStream) != -1) {
      line[strlen(line) - 1] = '\0';
      if ((ignoreLetterCasing && (strcasestr(line, keyword) != NULL)) ||
          (strstr(line, keyword) != NULL)) {
        fprintf(outputStream, "%s\n", line);
      }
    }
    free(line);

    if (fclose(inputStream) == EOF) {
      error("fclose");
    }
  } while (++optind < argc);

  if (fclose(outputStream) == EOF) {
    error("fclose");
  }

  exit(EXIT_SUCCESS);
}
