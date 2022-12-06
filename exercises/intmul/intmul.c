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

#define READ_END (0)   // pipefd[0] meaning
#define WRITE_END (1)  // pipefd[1] meaning

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
 *       + aH · bH · 16^n       -> done by HH (index 0)
 *       + aH · bL · 16^(n/2)   -> done by HL (index 1)
 *       + aL · bH · 16^(n/2)   -> done by LH (index 2)
 *       + aL · bL              -> done by LL (index 3)
 *
 * Each of the 4 children requires 2 pipes.
 * The pipefd array has the file descriptors for our 8 pipes.
 *
 *    PARENT_TO_CHILD pipes  -> parent reads, child writes (child-index * 2)
 *    CHILD_TO_PARENT pipes  -> parent writes, child reads (child-index * 2 + 1)
 *
 * Write into the P2C pipes and wait for a response in the C2P pipes.
 * Then add the 4 responses from the children together and print on stdout.
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

  // split input into 4 parts
  HexStringQuad quad = getHexStringQuad(pair);
  printf("a: %s\n", pair.a);
  printf("b: %s\n", pair.b);
  printf("a pair: %s %s\n", quad.aH, quad.aL);
  printf("b pair: %s %s\n", quad.bH, quad.bL);

  // open 8 pipes
  int pipefd[8][2];
  bool hasFailed = false;
  for (int i = 0; i < 8; i++) {
    hasFailed = hasFailed || (pipe(pipefd[i]) == -1);
  }
  if (hasFailed) {
    error("pipe");
  }

  // spawn 4 children
  pid_t pid[4];
  for (int i = 0; i < 4; i++) {
    pid[i] = fork();
    if (pid[i] == -1) {
      error("fork");
    } else if (pid[i] == 0) {  // spawned child continues here
      for (int j = 0; j < 8; j++) {
        bool PARENT_TO_CHILD = (i % 2 == 0);
        bool CHILD_TO_PARENT = (i % 2 != 0);

        // p2c read end -> child's stdin
        if (PARENT_TO_CHILD) {
          if (dup2(pipefd[j][READ_END], STDIN_FILENO) == -1) {
            error("dup2");
          }
        }

        // c2p write end -> child's stdout
        if (CHILD_TO_PARENT) {
          if (dup2(pipefd[j][WRITE_END], STDOUT_FILENO) == -1) {
            error("dup2");
          }
        }

        // close everything
        if (close(pipefd[j][0]) == -1 || close(pipefd[j][0]) == -1) {
          error("close");
        }
      }

      // run intmul
      execlp("./intmul", "./intmul", NULL);
      error("execlp");
    }
  }

  for (int j = 0; j < 8; j++) {
    bool PARENT_TO_CHILD = (i % 2 == 0);
    bool CHILD_TO_PARENT = (i % 2 != 0);

    // close p2c read ends
    if (PARENT_TO_CHILD) {
      if (close(pipefd[j][READ_END]) == -1) {
        error("close");
      }
    }

    // close c2p write ends
    if (CHILD_TO_PARENT) {
      if (close(pipefd[j][WRITE_END]) == -1) {
        error("close");
      }
    }
  }

  free(quad.aH);
  free(quad.aL);
  free(quad.bH);
  free(quad.bL);

  free(pair.a);
  free(pair.b);

  exit(EXIT_SUCCESS);
}