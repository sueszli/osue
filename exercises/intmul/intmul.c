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
} StringPair;

typedef struct {
  char* aH;
  char* aL;
  char* bH;
  char* bL;
  size_t len;
} StringQuad;

#pragma region "reliable"
static void addChars(char** strp, size_t num, char c, bool addToStart) {
  // if !addToStart, then it will add it to the end of strp
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

static StringPair getInput(void) {
  // post-condition: free returned StringPair

  StringPair pair;
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

static StringQuad splitToQuad(StringPair pair) {
  // post-condition: free returned quad

  StringQuad quad;
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
#pragma endregion "reliable"

static char* addHexStrings(char* str1, char* str2) {
  // post-condition: free output

  return "test";
}

int main(int argc, char* argv[]) {
  StringPair pair = getInput();
  StringQuad quad = splitToQuad(pair);

  printf("a-pair before: %s\n", pair.a);
  addChars(&(quad.aH), 1, 'X', false);
  addChars(&(quad.aH), 1, 'X', true);

  printf("a-pair after: %s\n", pair.a);
  printf("changed aH: %s\n", quad.aH);

  free(pair.a);
  free(pair.b);

  free(quad.aH);
  free(quad.aL);
  free(quad.bH);
  free(quad.bL);

  exit(EXIT_SUCCESS);
}

int main2(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }

  // base case
  StringPair pair = getInput();
  if (pair.len == 1) {
    errno = 0;
    fprintf(stdout, "%lx\n",
            strtoul(pair.a, NULL, 16) * strtoul(pair.b, NULL, 16));
    fflush(stdout);
    if (errno != 0) {
      error("strtoull");
    }
    free(pair.a);
    free(pair.b);
    exit(EXIT_SUCCESS);
  }

  enum child_index { aH_bH = 0, aH_bL = 1, aL_bH = 2, aL_bL = 3 };
  enum pipe_end { READ_END = 0, WRITE_END = 1 };

  // open 8 pipes
  int parent2child[4][2];
  int child2parent[4][2];
  for (int i = 0; i < 4; i++) {
    if ((pipe(parent2child[i]) == -1) || (pipe(child2parent[i]) == -1)) {
      error("pipe");
    }
    if ((close(parent2child[i][READ_END]) == -1) ||
        (close(child2parent[i][WRITE_END]) == -1)) {
      error("close");
    }
  }

  // fork 4 children
  pid_t cpid[4];
  for (int i = 0; i < 4; i++) {
    cpid[i] = fork();
    if (cpid[i] == -1) {
      error("fork");
    }
    if (cpid[i] == 0) {
      // child redirects stdin and stdout (fork duplicates pipes for child)
      for (int j = 0; j < 4; i++) {
        if ((dup2(parent2child[j][READ_END], STDIN_FILENO) == -1) ||
            (dup2(child2parent[j][WRITE_END], STDOUT_FILENO) == -1)) {
          error("dup2");
        }
        if ((close(parent2child[j][READ_END]) == -1) ||
            (close(parent2child[j][WRITE_END]) == -1) ||
            (close(child2parent[j][READ_END]) == -1) ||
            (close(child2parent[j][WRITE_END]) == -1)) {
          error("close");
        }
      }

      // child runs intmul (waits for arguments in stdin)
      execlp("./intmul", "./intmul", NULL);
      error("execlp");
    }
  }

  // write into p2c pipes
  StringQuad quad = splitToQuad(pair);
  free(pair.a);
  free(pair.b);
  for (int i = 0; i < 4; i++) {
    char* arg1;
    char* arg2;
    switch (i) {
      case aH_bH:
        arg1 = quad.aH;
        arg2 = quad.bH;
        break;
      case aH_bL:
        arg1 = quad.aH;
        arg2 = quad.bL;
        break;
      case aL_bH:
        arg1 = quad.aL;
        arg2 = quad.bH;
        break;
      case aL_bL:
        arg1 = quad.aL;
        arg2 = quad.bL;
        break;
    }
    addChars(&arg1, 1, '\n', false);
    addChars(&arg2, 1, '\n', false);
    if (((write(parent2child[i][WRITE_END], arg1, quad.len) == -1) ||
         (write(parent2child[i][WRITE_END], arg2, quad.len) == -1)) &&
        (errno != EINTR)) {
      error("write");
    }
    if (close(parent2child[i][WRITE_END]) == -1) {
      error("close")
    }
  }
  free(quad.aH);
  free(quad.aL);
  free(quad.bH);
  free(quad.bL);

  // wait for child to exit
  for (int i = 0; i < 4; i++) {
    int status;
    if (waitpid(cpid[i], &status, 0) == -1) {
      error("waitpid");
    }
    if (WEXITSTATUS(status) == EXIT_FAILURE) {
      error("child failed");
    }
  }

  // read from c2p pipes
  char* childResult[4];
  for (int i = 0; i < 4; i++) {
    size_t len;

    // can i write into fp
    FILE* stream = fdopen(child2parent[i][READ_END], "r");
    if (stream == NULL) {
      error("fdopen");
    }
    if (getline(&childResult[i], &len, stream) == -1) {
      error("getline");
    }
    addChars(&childResult[i], 1, '\0', false);
    if (close(child2parent[i][READ_END]) == -1) {
      error("close");
    }
    if (fclose(stream) == -1) {
      error("fclose");
    }
  }

  // shift and calculate sum
  const int n = pair.len;
  addChars(&childResult[aH_bH], n, '0', false);
  addChars(&childResult[aH_bL], n / 2, '0', false);
  addChars(&childResult[aL_bH], n / 2, '0', false);

  // char* sum = NULL;
  // for (int i = 0; i < 4; i++) {
  //   addHexStrings(&sum, &childResult[i]);
  //   free(childResult[i]);
  // }
  // fprintf(stdout, "%s\n", sum);
  // fflush(stdout);
  // free(sum);

  exit(EXIT_SUCCESS);
}