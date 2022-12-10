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

  // - ./client [-a optargA | -b optargB | -o ] -c [optargC] file... (max. 8
  // files)
  // - hint: optargC can only be specified by -coptargC (not -c optargC)
  // - optargA ... int [-50,300]
  // - optargB ... char
  // - optargC ... char[8] (exactly!)
  // - maximum of 8 pos-args, no minimum, cat all to one total_string

  int a = -51;

  if (a != -51)
    printf("a: %d\n", a);
  else
    printf("a: not initialized yet.\n");
  printf("b: %s\n", b);
  printf("c: %s\n", c);
  printf("o_set: %s\n", o_set ? "true" : "false");
  printf("total_string: %s\n", total_string);

  return 0;
}

/** Prints the usage message and exits with EXIT_FAILURE. */
void usage(const char *message) {
  fprintf(stderr, "%s\n", message);
  fprintf(stderr,
          "Usage: %s [-a optargA | -b optargB | -o ] -c [optargC] file... "
          "(max. 8 files)\n",
          program_name);
  exit(EXIT_FAILURE);
}
