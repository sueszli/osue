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

#pragma region done

static void addChars(char** strp, size_t num, char c, bool addToStart) {
  // if !addToStart, then it will add it to the start of strp
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

  if (addToStart) {
    memset(rStr, c, num);
    memcpy((rStr + num), oldCopy, oldSize);
  } else {
    memcpy(rStr, oldCopy, oldSize - 1);
    memset((rStr + oldSize - 1), c, num);
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
    usage("too few arguments");
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
    addChars(&(pair.a), diff, '0', true);
  } else if (len1 > len2) {
    addChars(&(pair.b), diff, '0', true);
  }
  assert(strlen(pair.a) == strlen(pair.b));

  // get len to be a power of 2
  const size_t len = strlen(pair.a);
  if ((len & (len - 1)) != 0) {
    const size_t newLen = 1 << (size_t)ceill(log2l((long double)len));
    addChars(&(pair.a), newLen - len, '0', true);
    addChars(&(pair.b), newLen - len, '0', true);
  }

  pair.len = strlen(pair.a);
  return pair;
}

static HexStringQuad getHexStringQuad(HexStringPair pair) {
  // post-condition: free returned quad

  HexStringQuad quad;
  size_t size = (pair.len / 2) + 1;

  quad.aH = malloc(size * sizeof(char));
  quad.bH = malloc(size * sizeof(char));
  quad.aL = malloc(size * sizeof(char));
  quad.bL = malloc(size * sizeof(char));

  memcpy(quad.aH, pair.a, size - 1);  // high digits (left side)
  memcpy(quad.bH, pair.b, size - 1);
  memcpy(quad.aL, (pair.a + size - 1), size - 1);  // low digits (right side)
  memcpy(quad.bL, (pair.b + size - 1), size - 1);

  quad.aH[size - 1] = '\0';
  quad.bH[size - 1] = '\0';
  quad.aL[size - 1] = '\0';
  quad.bL[size - 1] = '\0';

  quad.len = size - 1;
  return quad;
}

#pragma endregion done

int main(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }

  // base case
  HexStringPair pair = getInput();
  printf("a: %s\n", pair.a);
  printf("b: %s\n", pair.b);
  if (pair.len == 1) {
    errno = 0;
    printf("%lx\n", strtoul(pair.a, NULL, 16) * strtoul(pair.b, NULL, 16));
    if (errno != 0) {
      error("strtoull");
    }
    free(pair.a);
    free(pair.b);
    exit(EXIT_SUCCESS);
  }

  // split input into 4 parts
  HexStringQuad quad = getHexStringQuad(pair);
  printf("aH: %s\n", quad.aH);
  printf("aL: %s\n", quad.aL);
  printf("bH: %s\n", quad.bH);
  printf("bL: %s\n", quad.bL);

  /*
  // open 8 pipes
  int pipefd[8][2];
  for (int i = 0; i < 8; i++) {
    if (pipe(pipefd[i]) == -1) {
      error("pipe");
    }
  }
  enum pipefd_index {
    p2c_HH = 0,  // parent to child
    p2c_HL = 1,
    p2c_LH = 2,
    p2c_LL = 3,
    c2p_HH = 4,  // child to parent
    c2p_HL = 5,
    c2p_LH = 6,
    c2p_LL = 7
  };
  enum pipefd_end { READ_END = 0, WRITE_END = 1 };

  // close p2c read-ends
  if (close(pipefd[p2c_HH][READ_END]) == -1) error("close");
  if (close(pipefd[p2c_HL][READ_END]) == -1) error("close");
  if (close(pipefd[p2c_LH][READ_END]) == -1) error("close");
  if (close(pipefd[p2c_LL][READ_END]) == -1) error("close");

  // close c2p write-ends
  if (close(pipefd[c2p_HH][WRITE_END]) == -1) error("close");
  if (close(pipefd[c2p_HL][WRITE_END]) == -1) error("close");
  if (close(pipefd[c2p_LH][WRITE_END]) == -1) error("close");
  if (close(pipefd[c2p_LL][WRITE_END]) == -1) error("close");

  // spawn 4 children
  pid_t pid[4];
  enum pid_index { HH = 0, HL = 1, LH = 2, LL = 3 };
  for (int i = 0; i < 4; i++) {
    pid[i] = fork();  // also duplicates the pipes for child

    if (pid[i] == -1) {
      error("fork");
    }

    if (pid[i] == 0) {  // < child continues here
      for (int j = 0; j < 8; j++) {
        // p2c-read-end == child's stdin
        if (j <= p2c_LL) {
          if (dup2(pipefd[j][READ_END], STDIN_FILENO) == -1) {
            error("dup2");
          }
        }

        // c2p-write-end == child's stdout
        if (j >= c2p_HH) {
          if (dup2(pipefd[j][WRITE_END], STDOUT_FILENO) == -1) {
            error("dup2");
          }
        }

        // close all
        if (close(pipefd[j][READ_END]) == -1) error("close");
        if (close(pipefd[j][WRITE_END]) == -1) error("close");
      }

      // run intmul
      execlp("./intmul", "./intmul", NULL);
      error("execlp");
    }
  }

  // write into p2c HH
  if ((write(pipefd[p2c_HH][WRITE_END], quad.aH, quad.len) == -1) &&
      (errno != EINTR))
    error("write");
  if ((write(pipefd[p2c_HH][WRITE_END], quad.bH, quad.len) == -1) &&
      (errno != EINTR))
    error("write");
  if (close(pipefd[p2c_HH][WRITE_END]) == -1) error("close");

  // write into p2c HL
  if ((write(pipefd[p2c_HL][WRITE_END], quad.aH, quad.len) == -1) &&
      (errno != EINTR))
    error("write");
  if ((write(pipefd[p2c_HL][WRITE_END], quad.bL, quad.len) == -1) &&
      (errno != EINTR))
    error("write");
  if (close(pipefd[p2c_HL][WRITE_END]) == -1) error("close");

  // write into p2c LH
  if ((write(pipefd[p2c_LH][WRITE_END], quad.aL, quad.len) == -1) &&
      (errno != EINTR))
    error("write");
  if ((write(pipefd[p2c_LH][WRITE_END], quad.bH, quad.len) == -1) &&
      (errno != EINTR))
    error("write");
  if (close(pipefd[p2c_LH][WRITE_END]) == -1) error("close");

  // write into p2c LL
  if ((write(pipefd[p2c_LL][WRITE_END], quad.aL, quad.len) == -1) &&
      (errno != EINTR))
    error("write");
  if ((write(pipefd[p2c_LL][WRITE_END], quad.bL, quad.len) == -1) &&
      (errno != EINTR))
    error("write");
  if (close(pipefd[p2c_LL][WRITE_END]) == -1) error("close");

  // wait
  int status[4];
  if (waitpid(pid[HH], &status[HH], 0) == -1) error("waitpid");
  if (waitpid(pid[HL], &status[HL], 0) == -1) error("waitpid");
  if (waitpid(pid[LH], &status[LH], 0) == -1) error("waitpid");
  if (waitpid(pid[LL], &status[LL], 0) == -1) error("waitpid");
  if (WEXITSTATUS(status[HH]) != 0 || WEXITSTATUS(status[HL]) != 0 ||
      WEXITSTATUS(status[LH]) != 0 || WEXITSTATUS(status[LL]) != 0) {
    error("child didn't return EXIT_SUCCESS");
  }

  // read from c2p
  HexStringQuad childOutput;
  */

  // free resources
  free(pair.a);
  free(pair.b);

  free(quad.aH);
  free(quad.aL);
  free(quad.bH);
  free(quad.bL);

  exit(EXIT_SUCCESS);
}