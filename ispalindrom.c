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

// region "print formatting"
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

static char *trim(char *line) {
  // https://stackoverflow.com/questions/74350465/how-to-free-memory-following-a-0-placed-somewhere-in-a-string-in-c
  char *out = strdup(line);

  char *op = out;
  char *ip = line;
  while (*ip != '\0') {
    if (!isspace(*ip)) {
      *op = *ip;
      op++;
    }
    ip++;
  }
  *op = '\0';

  size_t newLen = sizeof(char) * (strlen(out) + 1);
  char *tmp = (char *)realloc(out, newLen);
  if (tmp == NULL) {
    error("realloc failed");
  }
  out = tmp;

  return out;
}

static char *toLowerCase(char *line) {
  char *out = strdup(line);

  char *p = out;
  while (*p != '\0') {
    *p = tolower(*p);
    p++;
  }
  return out;
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

static void processLine(char *line, uint8_t ignoreWhitespaces,
                        uint8_t ignoreLetterCasing, char *outputPath) {
  // change line based on options
  if (ignoreWhitespaces) {
    char *tmp = trim(line);
    free(line);
    line = tmp;
  }
  if (ignoreLetterCasing) {
    char *tmp = toLowerCase(line);
    free(line);
    line = tmp;
  }

  // add message to line
  char *msg = (isPalindrome(line) ? " is a palindrom" : " is not a palindrom");
  // printf("\"%s\"%s\n", line, msg);

  size_t newLen = sizeof(char) * (strlen(line) + strlen(msg)) + 1;
  char *tmp = (char *)realloc(line, newLen);
  if (tmp == NULL) {
    error("realloc failed");
  }
  line = tmp;

  line = strcat(line, msg);
  // free(msg);

  printf("%s\n", line);
  if (outputPath != NULL) {
  } else {
  }
  // free(line);
}

int main(int argc, char **argv) {
  uint8_t ignoreLetterCasing = 0;
  uint8_t ignoreWhitespaces = 0;
  uint8_t writeToFile = 0;
  char *outputPath;

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

  const int numInputFiles = argc - optind;
  log("%s %d", "num of input files:", numInputFiles);

  if (numInputFiles > 0) {
    // read files
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
        processLine(line, ignoreWhitespaces, ignoreLetterCasing, outputPath);
      }
      fclose(inputStream);
    }

  } else {
    // read user input
    char *line = NULL;
    size_t len = 0;
    if (getline(&line, &len, stdin) == -1) {
      error("reading from stdin with getline failed");
    }
    processLine(line, ignoreWhitespaces, ignoreLetterCasing, outputPath);
  }

  return EXIT_SUCCESS;
}
