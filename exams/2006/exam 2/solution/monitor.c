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

static int writeToDatabase(char* msg) {
  // stores data in mock database (could be writing into a file or anything ...)
  int status = printf("database received: %s\n", msg);
  return (status == -1 ? -1 : 0);
}

/**
 * I ignored the arguments and just used "echo" and "cat" for testing purposes.
 * The only thing that should be changed are the excl calls in the child
 * processes.
 */
int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage("invalid number of arguments");
  }

  enum pipe_ends { READ, WRITE };

  // prog.stderr --> database
  int stderr2database[2];
  if ((pipe(stderr2database) == -1)) {
    error("pipe");
  }

  // prog.stdout --> log.stdin
  int stdout2log[2];
  if (pipe(stdout2log) == -1) {
    error("pipe");
  }

  // run prog
  pid_t progPid = fork();
  if (progPid == -1) {
    error("fork");

  } else if (progPid == 0) {
    if (dup2(stderr2database[WRITE], fileno(stderr)) == -1) {
      error("dup");
    }
    if (dup2(stdout2log[WRITE], fileno(stdout)) == -1) {
      error("dup");
    }
    close(stderr2database[READ]);
    close(stderr2database[WRITE]);
    close(stdout2log[READ]);
    close(stdout2log[WRITE]);

    // TODO: should be execlp(./prog) instead of "echo"
    fprintf(stderr, "STDERR MESSAGE FROM PROG!");
    execl("/bin/echo", "echo", "-e", "STDOUT MESSAGE FROM PROG!", (char*)NULL);
    error("execl");
  }

  // run log
  pid_t logPid = fork();
  if (logPid == -1) {
    error("fork");

  } else if (logPid == 0) {
    if (dup2(stdout2log[READ], fileno(stdin)) == -1) {
      error("dup");
    }
    close(stderr2database[READ]);
    close(stderr2database[WRITE]);
    close(stdout2log[READ]);
    close(stdout2log[WRITE]);

    // TODO: should be execlp(./log) instead of "cat"
    execl("/bin/sh", "sh", "-c", "cat | sed 's/^/log received: /'",
          (char*)NULL);
    error("execl");
  }

  // continue as parent
  close(stderr2database[WRITE]);
  close(stdout2log[READ]);
  close(stdout2log[WRITE]);

  char buf[BUFFER_SIZE];
  memset(&buf, (int)'\0', BUFFER_SIZE);
  int progStatus;
  int logStatus;

  while (true) {
    read(stderr2database[READ], &buf, BUFFER_SIZE);
    if (writeToDatabase(buf) == -1) {
      error("writing to mock database failed");
    }

    waitpid(progPid, &progStatus, WUNTRACED | WCONTINUED);
    bool progAlive = !WIFEXITED(progStatus) && !WIFSIGNALED(progStatus);

    waitpid(logPid, &logStatus, WUNTRACED | WCONTINUED);
    bool logAlive = !WIFEXITED(logStatus) && !WIFSIGNALED(logStatus);

    if (!progAlive && !logAlive) {
      break;
    }
  }
  close(stderr2database[READ]);

  // check childrens exit codes
  if (WEXITSTATUS(progStatus) == EXIT_FAILURE) {
    error("error in a prog childprocess");
  }
  if (WEXITSTATUS(logStatus) == EXIT_FAILURE) {
    error("error in a log childprocess");
  }

  exit(EXIT_SUCCESS);
}