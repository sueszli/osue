#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define usage(msg)                                               \
  do {                                                           \
    fprintf(stderr, "Usage: %s\nSYNOPSIS:\n\tmonitor prog log"); \
    exit(EXIT_FAILURE);                                          \
  } while (0)

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage("invalid number of arguments");
  }
  char* prog = argv[1];
  char* log = argv[2];
}