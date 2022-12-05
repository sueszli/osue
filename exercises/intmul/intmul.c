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

  return hexStringPair;
}

static void generateLeadingZeroes(char** strp, size_t num) {
  // pre-condition: str must be allocated dynamically
  // side-effect: adds leading zeroes to str and reallocates it

  size_t oldSize = strlen(*strp) + 1;
  char* oldCopy = strdup(*strp);
  if (oldCopy == NULL) {
    error("strdup");
  }
  char* rStr = realloc(*strp, (oldSize + num) * sizeof(char));
  if (rStr == NULL) {
    error("realloc");
  }

  for (size_t i = 0; i < num; i++) {
    rStr[i] = '0';
  }
  for (size_t i = 0; i < oldSize; i++) {
    rStr[i + num] = oldCopy[i];
  }
  *strp = rStr;
  free(oldCopy);
}

static void validateInput(HexStringPair* pair) {
  // side-effect: adds leading zeros such that pair has same length

  const char* valid = "0123456789abcdefABCDEF";
  if ((strspn(pair->hex1, valid) != strlen(pair->hex1)) ||
      (strspn(pair->hex2, valid) != strlen(pair->hex2))) {
    free(pair->hex1);
    free(pair->hex2);
    usage("one argument is not a hexadecimal number")
  }

  // get both args to same size
  const size_t diff =
      (size_t)labs((long)(strlen(pair->hex2) - strlen(pair->hex1)));
  const bool increaseHex1 = strlen(pair->hex1) < strlen(pair->hex2);
  const bool increaseHex2 = strlen(pair->hex1) > strlen(pair->hex2);
  if (increaseHex1) {
    generateLeadingZeroes(&(pair->hex1), diff);
  } else if (increaseHex2) {
    generateLeadingZeroes(&(pair->hex2), diff);
  }
  pair->len = strlen(pair->hex1);

  // make arg length divisible by 2
  if (pair->len % 2 != 0) {
    generateLeadingZeroes(&(pair->hex1), 1);
    generateLeadingZeroes(&(pair->hex2), 1);
    pair->len = (pair->len) + 1;
  }
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }
  HexStringPair hexStringPair = getInput();
  printf("hex1 before validation: %s\n", hexStringPair.hex1);
  printf("hex2 before validation: %s\n", hexStringPair.hex2);

  validateInput(&hexStringPair);
  printf("hex1 after validation: %s\n", hexStringPair.hex1);
  printf("hex2 after validation: %s\n", hexStringPair.hex2);
  printf("len: %ld\n", hexStringPair.len);

  free(hexStringPair.hex1);
  free(hexStringPair.hex2);
  return EXIT_SUCCESS;
}