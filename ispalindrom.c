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

// functions have as little side effects as possible - but lower performance,
// because the input gets duplicated (dynamic memory allocation).
// Don't forget to free() the output of every function that returns a string.

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

static void writeUpdatedLine(char *line, uint8_t ignoreWhitespaces,
                             uint8_t ignoreLetterCasing, FILE *outputStream) {
  line[strlen(line) - 1] = '\0';  // remove '\n' at the end
  char *out = strdup(line);

  // get msg
  if (ignoreWhitespaces) {
    char *tmp = trim(out);
    free(out);
    out = tmp;
  }
  if (ignoreLetterCasing) {
    char *tmp = toLowerCase(out);
    free(out);
    out = tmp;
  }
  char *msg = isPalindrome(out) ? " is a palindrom\n" : " is not a palindrom\n";

  // out = line + msg
  size_t newLen = sizeof(char) * (strlen(out) + strlen(msg)) + 1;
  char *tmp = (char *)realloc(out, newLen);
  if (tmp == NULL) {
    error("realloc failed");
  }
  out = tmp;
  out = strcat(line, msg);  // don't free out -> it not points to line

  fprintf(outputStream, "%s", out);
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

  // read input
  const int numInputFiles = argc - optind;
  log("%s %d", "num of input files:", numInputFiles);
  if (numInputFiles > 0) {
    // read from file
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
      }
      free(line);
      fclose(inputStream);
    }
  } else {
    // read from stdin
    char *line = NULL;
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
