#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
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
  size_t lineLen = 0;
  ssize_t nChars;
  int nLines = 0;
  while ((nChars = getline(&line, &lineLen, stdin)) != -1) {
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
    usage("too few argument");
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
  const size_t len1 = strlen(pair.hex1);
  const size_t len2 = strlen(pair.hex2);
  const size_t diff = (size_t)labs((long)(len2 - len1));
  if (len1 < len2) {
    addLeadingZeroes(&(pair.hex1), diff);
  } else if (len1 > len2) {
    addLeadingZeroes(&(pair.hex2), diff);
  }
  assert(strlen(pair.hex1) == strlen(pair.hex2));
  const size_t len = strlen(pair.hex1);

  // get len to be a power of 2
  if ((len & (len - 1)) != 0) {
    const size_t newLen = 1 << (size_t)ceill(log2l((long double)len));
    addLeadingZeroes(&(pair.hex1), newLen - len);
    addLeadingZeroes(&(pair.hex2), newLen - len);
  }

  pair.len = strlen(pair.hex1);
  return pair;
}

static unsigned long long parseHexStr(char* hexStr) {
  errno = 0;  // make errors visible
  char* endptr = NULL;
  unsigned long long out = strtoull(hexStr, &endptr, 16);
  if ((errno == ERANGE) || (errno == EINVAL) || (endptr == hexStr)) {
    error("strtoull");
  }
  return out;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }
  HexStringPair pair = getInput();

  printf("hex1: %s\n", pair.hex1);
  printf("hex2: %s\n", pair.hex2);

  if (pair.len == 1) {
    // base case
    unsigned long long fst = parseHexStr(pair.hex1);
    unsigned long long snd = parseHexStr(pair.hex2);
    unsigned long long result = fst * snd;
    fprintf(stdout, "%llx\n", result);
    fflush(stdout);

  } else {
    // call self recursively
  }

  free(pair.hex1);
  free(pair.hex2);
  return EXIT_SUCCESS;
}