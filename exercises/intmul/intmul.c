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

#define usage(msg)                                                   \
  do {                                                               \
    fprintf(stderr, "Invalid input: %s\nSYNOPSIS: ./intmul\n", msg); \
    exit(EXIT_FAILURE);                                              \
  } while (true);

typedef struct {
  char* hex1;
  char* hex2;
  size_t len;
} HexStringPair;

static void addLeadingZeroes(char** strp, size_t num) {
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
  memset(rStr, '0', num);
  memcpy((rStr + num), oldCopy, oldSize);
  free(oldCopy);
  *strp = rStr;
}

static HexStringPair getInput(void) {
  // post-condition: free returned HexStringPair

  HexStringPair pair;

  char* line = NULL;
  size_t len = 0;
  ssize_t nChars;
  int nLines = 0;
  while ((nChars = getline(&line, &len, stdin)) != -1) {
    if (nLines == 0) {
      line[strlen(line) - 1] = '\0';
      pair.hex1 = strdup(line);
    } else if (nLines == 1) {
      line[strlen(line) - 1] = '\0';
      pair.hex2 = strdup(line);
    }
    nLines++;
  }
  free(line);

  // validate input
  if (nLines == 1) {
    free(pair.hex1);
    usage("missing 1 argument");
  }
  if (nLines > 2) {
    free(pair.hex1);
    free(pair.hex2);
    usage("too many arguments");
  }
  if ((strlen(pair.hex1) == 0) || (strlen(pair.hex2) == 0)) {
    free(pair.hex1);
    free(pair.hex2);
    usage("empty argument");
  }
  const char* valid = "0123456789abcdefABCDEF";
  if ((strspn(pair.hex1, valid) != strlen(pair.hex1)) ||
      (strspn(pair.hex2, valid) != strlen(pair.hex2))) {
    free(pair.hex1);
    free(pair.hex2);
    usage("one argument is not a hexadecimal number")
  }

  // get len to the same size in pair
  const size_t diff =
      (size_t)labs((long)(strlen(pair.hex2) - strlen(pair.hex1)));
  const bool increaseHex1 = strlen(pair.hex1) < strlen(pair.hex2);
  const bool increaseHex2 = strlen(pair.hex1) > strlen(pair.hex2);
  if (increaseHex1) {
    addLeadingZeroes(&(pair.hex1), diff);
  } else if (increaseHex2) {
    addLeadingZeroes(&(pair.hex2), diff);
  }

  // make len divisible by 2
  if (strlen(pair.hex1) % 2 != 0) {
    addLeadingZeroes(&(pair.hex1), 1);
    addLeadingZeroes(&(pair.hex2), 1);
  }
  pair.len = strlen(pair.hex1);
  return pair;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }
  HexStringPair hexStringPair = getInput();

  printf("hex1: %s\n", hexStringPair.hex1);
  printf("hex2: %s\n", hexStringPair.hex2);

  free(hexStringPair.hex1);
  free(hexStringPair.hex2);
  return EXIT_SUCCESS;
}