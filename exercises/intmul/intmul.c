#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

typedef struct {
  char* hex1H;
  char* hex1L;
  char* hex2H;
  char* hex2L;
  size_t len;
} HexStringQuad;

static void addZeroes(char** strp, size_t num, bool leading) {
  // if !leading, then trailing
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

  if (leading) {
    memset(rStr, '0', num);
    memcpy((rStr + num), oldCopy, oldSize);
  } else {
    memcpy(rStr, oldCopy, oldSize - 1);
    memset((rStr + oldSize - 1), '0', num);
    rStr[oldSize - 1 + num] = '\0';
  }

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

  // get len to the same in pair
  const size_t len1 = strlen(pair.hex1);
  const size_t len2 = strlen(pair.hex2);
  const size_t diff = (size_t)labs((long)(len2 - len1));
  if (len1 < len2) {
    addZeroes(&(pair.hex1), diff, true);
  } else if (len1 > len2) {
    addZeroes(&(pair.hex2), diff, true);
  }
  assert(strlen(pair.hex1) == strlen(pair.hex2));

  // get len to be a power of 2
  const size_t len = strlen(pair.hex1);
  if ((len & (len - 1)) != 0) {
    const size_t newLen = 1 << (size_t)ceill(log2l((long double)len));
    addZeroes(&(pair.hex1), newLen - len, true);
    addZeroes(&(pair.hex2), newLen - len, true);
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

static HexStringQuad getHexStringQuad(HexStringPair pair) {
  // post-condition: free returned quad

  HexStringQuad quad;
  size_t size = pair.len / 2 + 1;

  // high digits
  quad.hex1H = malloc(size * sizeof(char));
  quad.hex2H = malloc(size * sizeof(char));
  memcpy(quad.hex1H, pair.hex1, size - 1);
  memcpy(quad.hex2H, pair.hex2, size - 1);
  quad.hex1H[size - 1] = '\0';
  quad.hex2H[size - 1] = '\0';

  // low digits
  quad.hex1L = malloc(size * sizeof(char));
  quad.hex2L = malloc(size * sizeof(char));
  memcpy(quad.hex1L, (pair.hex1 + size - 1), size);
  memcpy(quad.hex2L, (pair.hex2 + size - 1), size);

  quad.len = size - 1;
  return quad;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }

  // read input
  HexStringPair pair = getInput();
  printf("hex1 pair: %s\n", pair.hex1);
  printf("hex2 pair: %s\n", pair.hex2);
  printf("len: %ld\n\n", pair.len);

  // base case
  if (pair.len == 1) {
    unsigned long long result = parseHexStr(pair.hex1) * parseHexStr(pair.hex2);
    fprintf(stdout, "RESULT: %llx\n", result);
    fflush(stdout);

    free(pair.hex1);
    free(pair.hex2);
    return EXIT_SUCCESS;
  }

  // split input into 4 parts to pass to children
  /*
    [aH aL] * [bH bL] =
      + aH * bH * 16^n       -> sent to HH child
      + aH * bL * 16^(n/2)   -> sent to HL child
      + aL * bH * 16^(n/2)   -> sent to LH child
      + aL * bL              -> sent to LL child
  */
  HexStringQuad quad = getHexStringQuad(pair);
  printf("hex1 quad: %s %s\n", quad.hex1H, quad.hex1L);
  printf("hex2 quad: %s %s\n", quad.hex2H, quad.hex2L);
  printf("len: %ld\n", quad.len);

  // open 2 pipes per child
  int pipefd[8][2];
  const int READ_1H = 0;
  const int WRITE_1H = 1;
  const int READ_1L = 2;
  const int WRITE_1L = 3;
  const int READ_2H = 4;
  const int WRITE_2H = 5;
  const int READ_2L = 6;
  const int WRITE_2L = 7;
  if (pipe(pipefd[READ_1H]) == -1 || pipe(pipefd[WRITE_1H]) == -1 ||
      pipe(pipefd[READ_1L]) == -1 || pipe(pipefd[WRITE_1L]) == -1 ||
      pipe(pipefd[READ_2H]) == -1 || pipe(pipefd[WRITE_2H]) == -1 ||
      pipe(pipefd[READ_2L]) == -1 || pipe(pipefd[WRITE_2L]) == -1) {
    error("pipe");
  }

  // spawn 4 children

  // redirect pipes

  const int READ_END = 0;
  const int WRITE_END = 1;

  free(quad.hex1H);
  free(quad.hex1L);
  free(quad.hex2H);
  free(quad.hex2L);

  free(pair.hex1);
  free(pair.hex2);

  return EXIT_SUCCESS;
}