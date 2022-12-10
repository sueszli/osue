#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** name of the executable (for printing messages) */
char *program_name = "<not yet set>";

void usage(const char *message);

int main(int argc, char **argv) {
  /* set program_name */
  if (argc > 0) {
    program_name = argv[0];
  }

  // ./client {-a optargA | -b | -c [optargC] } file... (exactly 4 files)
  // - set flag, if -b occured
  // - add all four files in reversed mentioned-order to totalString

  printf("a: %s\n", a);
  printf("b_set: %s\n", b_set ? "true" : "false");
  printf("c: %s\n", c);
  printf("total_string: %s\n", total_string);

  return 0;
}

/** Prints the usage message and exits with EXIT_FAILURE. */
void usage(const char *message) {
  fprintf(stderr, "%s\n", message);
  fprintf(stderr,
          "Usage: %s { -a optargA | -b | -c [optargC] } file... (exactly 4 "
          "files)\n",
          program_name);
  exit(EXIT_FAILURE);
}
