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
  char* a;
  char* b;
  size_t len;
} HexStringPair;

typedef struct {
  char* aH;
  char* aL;
  char* bH;
  char* bL;
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
      pair.a = strdup(line);
    } else if (nLines == 1) {
      line[strlen(line) - 1] = '\0';
      pair.b = strdup(line);
    }
    nLines++;
  }
  free(line);

  // validate input
  if (nLines == 1) {
    free(pair.a);
    usage("too few argument");
  }
  if (nLines > 2) {
    free(pair.a);
    free(pair.b);
    usage("too many arguments");
  }
  if ((strlen(pair.a) == 0) || (strlen(pair.b) == 0)) {
    free(pair.a);
    free(pair.b);
    usage("empty argument");
  }
  const char* valid = "0123456789abcdefABCDEF";
  if ((strspn(pair.a, valid) != strlen(pair.a)) ||
      (strspn(pair.b, valid) != strlen(pair.b))) {
    free(pair.a);
    free(pair.b);
    usage("one argument is not a hexadecimal number")
  }

  // get len to the same in pair
  const size_t len1 = strlen(pair.a);
  const size_t len2 = strlen(pair.b);
  const size_t diff = (size_t)labs((long)(len2 - len1));
  if (len1 < len2) {
    addZeroes(&(pair.a), diff, true);
  } else if (len1 > len2) {
    addZeroes(&(pair.b), diff, true);
  }
  assert(strlen(pair.a) == strlen(pair.b));

  // get len to be a power of 2
  const size_t len = strlen(pair.a);
  if ((len & (len - 1)) != 0) {
    const size_t newLen = 1 << (size_t)ceill(log2l((long double)len));
    addZeroes(&(pair.a), newLen - len, true);
    addZeroes(&(pair.b), newLen - len, true);
  }

  pair.len = strlen(pair.a);
  return pair;
}

static unsigned long long parseHexStr(char* hexStr) {
  // note: this is an overkill for the base case: we only multiply one digit

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
  quad.aH = malloc(size * sizeof(char));
  quad.bH = malloc(size * sizeof(char));
  memcpy(quad.aH, pair.a, size - 1);
  memcpy(quad.bH, pair.b, size - 1);
  quad.aH[size - 1] = '\0';
  quad.bH[size - 1] = '\0';

  // low digits
  quad.aL = malloc(size * sizeof(char));
  quad.bL = malloc(size * sizeof(char));
  memcpy(quad.aL, (pair.a + size - 1), size);
  memcpy(quad.bL, (pair.b + size - 1), size);

  quad.len = size - 1;
  return quad;
}

/*
 * Base case: print product of single digit multiplication to stdout.
 * General case: create 4 children, each responsible for one product.
 *
 *    [aH aL] · [bH bL] =
 *       + aH · bH · 16^n       -> done by HH_child
 *       + aH · bL · 16^(n/2)   -> done by HL_child
 *       + aL · bH · 16^(n/2)   -> done by LH_child
 *       + aL · bL              -> done by LL_child
 */
int main(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }

  // base case
  HexStringPair pair = getInput();
  if (pair.len == 1) {
    fprintf(stdout, "%llx\n", parseHexStr(pair.a) * parseHexStr(pair.b));
    free(pair.a);
    free(pair.b);
    exit(EXIT_SUCCESS);
  }

  // general case
  HexStringQuad quad = getHexStringQuad(pair);
  printf("a: %s\n", pair.a);
  printf("b: %s\n", pair.b);
  printf("a pair: %s %s\n", quad.aH, quad.aL);
  printf("b pair: %s %s\n", quad.bH, quad.bL);

  // open 2 pipes per child (reading from child, writing to child)
  int pipefd[8][2];

  const int HH_READ = 0;
  const int HH_WRITE = 1;
  const int HL_READ = 2;
  const int HL_WRITE = 3;
  const int LH_READ = 4;
  const int LH_WRITE = 5;
  const int LL_READ = 6;
  const int LL_WRITE = 7;

  if (pipe(pipefd[HH_READ]) == -1 || pipe(pipefd[HH_WRITE]) == -1 ||
      pipe(pipefd[HL_READ]) == -1 || pipe(pipefd[HL_WRITE]) == -1 ||
      pipe(pipefd[LH_READ]) == -1 || pipe(pipefd[LH_WRITE]) == -1 ||
      pipe(pipefd[LL_READ]) == -1 || pipe(pipefd[LL_WRITE]) == -1) {
    error("pipe");
  }

  // spawn 4 children

  // redirect pipes

  const int READ_END = 0;
  const int WRITE_END = 1;

  free(quad.aH);
  free(quad.aL);
  free(quad.bH);
  free(quad.bL);

  free(pair.a);
  free(pair.b);

  exit(EXIT_SUCCESS);
}