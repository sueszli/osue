#include <errno.h>
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
  } while (0)

#define usage(msg)                                                    \
  do {                                                                \
    fprintf(stderr, "Usage: %s\nSYNOPSIS:\n\tmonitor prog log", msg); \
    exit(EXIT_FAILURE);                                               \
  } while (0)

#define READ (0)
#define WRITE (1)

static int storeErrorMsg(char* msg) {
  // stores data in mock database (could be writing into a file)
  if (msg != NULL) {
    return 0;
  }
  return -1;
}

static int closePipes(int p1[2], int p2[2]) {
  close(p1[READ]);
  close(p1[WRITE]);
  close(p2[READ]);
  close(p2[WRITE]);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage("invalid number of arguments");
  }
  const char* prog = argv[1];
  const char* log = argv[2];

  // prog.stderr --> database
  int error2database[2];
  if ((pipe(error2database) == -1)) {
    error("pipe");
  }

  // prog.stdout --> log.stdin
  int stdOut2logStdIn[2];
  if (pipe(stdOut2logStdIn) == -1) {
    error("pipe");
  }

  // run prog
  pid_t progPid = fork();
  if (progPid == -1) {
    error("fork");

  } else if (progPid == 0) {
    if (dup2(error2database[WRITE], fileno(stderr)) == -1) {
      error("dup");
    }
    if (dup2(stdOut2logStdIn[WRITE], fileno(stdout)) == -1) {
      error("dup");
    }
    closePipes(error2database, stdOut2logStdIn);

    execlp(prog, NULL);
    error("execlp");
  }

  // run log
  pid_t logPid = fork();
  if (logPid == -1) {
    error("fork");

  } else if (logPid == 0) {
    if (dup2(fileno(stdin), stdOut2logStdIn[READ]) == -1) {
      error("dup");
    }
    closePipes(error2database, stdOut2logStdIn);

    execlp(log, NULL);
    error("execlp");
  }

  // read output
  if (dup2(fileno(stdin), stdOut2logStdIn[READ]) == -1) {
    error("dup");
  }
  closePipes(error2database, stdOut2logStdIn);
}