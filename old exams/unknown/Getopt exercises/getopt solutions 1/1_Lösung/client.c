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

  /*

  - ./client [-a optargA] [-e] -c [optargC] [-b optargB [-d] ]

  - hint: optargC can only be specified by -coptargC (not -c optargC)

  - all optargs are char* (for simplicity)
  - hardest test-cases:
      - -d specified, although -b not specified (FAIL)
      - -d not specified, -b specified (PASS)

  */
  if (argc < 2) usage("Too few args.");
  if (argc > 9) usage("Too many args.");

  char *a = NULL;
  char *b = NULL;
  char *c = NULL;
  bool d_set = false;
  bool e_set = false;

  bool a_occured = false;
  bool b_occured = false;
  bool c_occured = false;
  bool d_occured = false;
  bool e_occured = false;

  char c1;
  while ((c1 = getopt(argc, argv, "a:b:c::de")) != -1) {
    switch (c1) {
      case 'a':
        if (a_occured) usage("passed in -a more than once.");
        a_occured = true;
        a = optarg;  // how to throw exception, if optargA not occured? ->
                     // getopt should do. Test it!
        break;
      case 'b':
        if (b_occured) usage("passed in -b more than once.");
        b_occured = true;
        b = optarg;  // how to throw exception, if optargB not occured? ->
                     // getopt should do. Test it!
        break;
      case 'c':
        if (c_occured) usage("passed in -c more than once.");
        c_occured = true;
        c = optarg;  // todo: test what happens, if optargC was not specified
        break;
      case 'd':
        if (d_occured) usage("passed in -d more than once.");
        d_occured = true;
        d_set = true;
        break;
      case 'e':
        if (e_occured) usage("passed in -e more than once.");
        e_occured = true;
        e_set = true;
        break;
      default:
        usage("(getopt detected an error)");
    }
  }

  if (!c_occured) usage("-c has to have occured.");
  if (d_occured && !b_occured) usage("-d only if -b occured.");

  printf("a: %s\n", a);
  printf("b: %s\n", b);
  printf("c: %s\n", c);
  printf("d_set: %s\n", d_set ? "true" : "false");
  printf("e_set: %s\n", e_set ? "true" : "false");

  return 0;
}

/** Prints the usage message and exits with EXIT_FAILURE. */
void usage(const char *message) {
  fprintf(stderr, "%s\n", message);
  fprintf(stderr,
          "Usage: %s [-a optargA] [-e] -c [optargC] [-b optargB [-d] ]\n",
          program_name);
  exit(EXIT_FAILURE);
}
