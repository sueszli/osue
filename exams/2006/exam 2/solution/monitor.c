#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
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

#define BUFFER_SIZE (80)

#define READ (0)
#define WRITE (1)

static int writeToDatabase(char* msg) {
  // stores data in mock database (could be writing into a file or anything ...)
  int status = printf("database received: %s\n", msg);
  return (status == -1 ? -1 : 0);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage("invalid number of arguments");
  }
  const char* prog = argv[1];
  const char* log = argv[2];
  printf("prog: %s log: %s\n", prog, log);

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

  printf(">>> arrived here!\n");

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
    close(error2database[READ]);
    close(error2database[WRITE]);
    close(stdOut2logStdIn[READ]);
    close(stdOut2logStdIn[WRITE]);

    execlp(prog, NULL);
    error("execlp");
  }

  // run log
  pid_t logPid = fork();
  if (logPid == -1) {
    error("fork");

  } else if (logPid == 0) {
    if (dup2(stdOut2logStdIn[READ], fileno(stdin)) == -1) {
      error("dup");
    }
    close(error2database[READ]);
    close(error2database[WRITE]);
    close(stdOut2logStdIn[READ]);
    close(stdOut2logStdIn[WRITE]);

    execlp(log, NULL);
    error("execlp");
  }

  // continue as parent
  close(error2database[WRITE]);
  close(stdOut2logStdIn[READ]);
  close(stdOut2logStdIn[WRITE]);

  char buf[BUFFER_SIZE];
  memset(&buf, (int)'\0', BUFFER_SIZE);
  int progStatus;
  int logStatus;

  while (true) {
    waitpid(progPid, &progStatus, WUNTRACED | WCONTINUED);
    bool progAlive = !WIFEXITED(progStatus) && !WIFSIGNALED(progStatus);

    waitpid(logPid, &logStatus, WUNTRACED | WCONTINUED);
    bool logAlive = !WIFEXITED(logStatus) && !WIFSIGNALED(logStatus);

    if (!logAlive || !progAlive) {
      break;
    }

    read(error2database[READ], &buf, BUFFER_SIZE);
    if (writeToDatabase(buf) != -1) {
      error("writing to mock database failed");
    }
  }
  close(error2database[READ]);

  // check childrens exit codes
  if (WEXITSTATUS(progStatus) == EXIT_FAILURE) {
    error("error in a prog childprocess");
  }
  if (WEXITSTATUS(logStatus) == EXIT_FAILURE) {
    error("error in a log childprocess");
  }

  exit(EXIT_SUCCESS);
}