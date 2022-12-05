#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (true);

#define usage(msg)                                                          \
  do {                                                                      \
    fprintf(stderr,                                                         \
            "Invalid input: %s\nCalculates the product of 2 hex numbers.\n" \
            "SYNOPSIS: ./intmul\n",                                         \
            msg);                                                           \
    exit(EXIT_FAILURE);                                                     \
  } while (true);

typedef struct {
  char* hex1;
  char* hex2;
  size_t len;  // same for both
} HexStringPair;

static char* addLeadingZeros(char* str, size_t diff) {
  // pre-condition: str must be dynamically allocated
  // post-condition: output must be freed (str must not be freed)

  size_t oldSize = strlen(str) + 1;
  size_t newSize = oldSize + diff;

  char* oldStrCopy = strdup(str);
  if (oldStrCopy == NULL) {
    error("strdup");
  }
  char* newStr = realloc(str, newSize * sizeof(*str));
  if (newStr == NULL) {
    error("realloc");
  }

  for (size_t i = 0; i < oldSize; i++) {
    newStr[i] = '0';
    newStr[i + diff] = oldStrCopy[i];
  }

  free(oldStrCopy);
  return newStr;
}

static void validateInput(HexStringPair pair) {
  // side-effect: adds leading zeros such that pair has same length

  const char* valid = "0123456789abcdefABCDEF";
  if ((strspn(pair.hex1, valid) != strlen(pair.hex1)) ||
      (strspn(pair.hex2, valid) != strlen(pair.hex2))) {
    free(pair.hex1);
    free(pair.hex2);
    usage("argument is not a hexadecimal number")
  }

  if (strlen(pair.hex1) < strlen(pair.hex2)) {
    size_t diff = strlen(pair.hex2) - strlen(pair.hex1);
    addLeadingZeros(pair.hex1, diff);

  } else if (strlen(pair.hex1) > strlen(pair.hex2)) {
    size_t diff = strlen(pair.hex1) - strlen(pair.hex2);
    addLeadingZeros(pair.hex2, diff);
  }

  // add leading zeros if their size is not 2^n

  pair.len = strlen(pair.hex1);
}

static HexStringPair getInput(void) {
  // post-condition: free returned HexStringPair

  HexStringPair hexStringPair;

  char* line = NULL;
  size_t len = 0;
  ssize_t nChars;
  int nLines = 0;
  while ((nChars = getline(&line, &len, stdin)) != -1) {
    if (nLines == 0) {
      line[strlen(line) - 1] = '\0';
      hexStringPair.hex1 = strdup(line);
      nLines++;
    } else if (nLines == 1) {
      line[strlen(line) - 1] = '\0';
      hexStringPair.hex2 = strdup(line);
      nLines++;
    } else {
      break;
    }
  }
  free(line);

  if (nLines == 1) {
    free(hexStringPair.hex1);
    usage("only 1 argument");
  }

  // validateInput(hexStringPair);
  return hexStringPair;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }
  HexStringPair hexStringPair = getInput();

  printf("hex1: %s\n", hexStringPair.hex1);
  printf("hex2: %s\n", hexStringPair.hex2);
  // printf("len: %d\n", hexStringPair.len);

  free(hexStringPair.hex1);
  free(hexStringPair.hex2);

  return EXIT_SUCCESS;
}