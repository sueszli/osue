#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *program_name = NULL;

void usage(const char *message) {
  fprintf(stderr, "%s\n", message);
  fprintf(stderr,
          "Usage: %s { -a optargA | -b | -c [optargC] } file... (exactly 4 "
          "files)\n",
          program_name);
  exit(EXIT_FAILURE);
}

/**
 * ./client {-a optargA | -b | -c [optargC] } file... (exactly 4 files)
 *    - set flag, if -b occured
 *    - add all four files in reversed mentioned-order to totalString
 */
int main(int argc, char **argv) {
  if (argc < 5) {
    usage("too few arguments");
  }
  if (argc > 6) {
    usage("too many arguments");
  }
  program_name = argv[0];

  bool optA = false;
  bool optB = false;
  bool optC = false;

  char *optargA = NULL;
  char *optargC = NULL;

  int opt = 0;
  while ((opt = getopt(argc, argv, "a:bc::")) != -1) {
    switch (opt) {
      case 'a':
        if (optA) {
          usage("same option occured more than once");
        }
        optA = true;
        if (optB || optC) {
          usage("did not use option a xor b xor c");
        }
        optargA = optarg;
        break;

      case 'b':
        if (optB) {
          usage("same option occured more than once");
        }
        optB = true;
        if (optA || optC) {
          usage("did not use option a xor b xor c");
        }
        break;

      case 'c':
        if (optC) {
          usage("same option occured more than once");
        }
        optC = true;
        if (optA || optB) {
          usage("did not use option a xor b xor c");
        }
        break;

      default:
        usage("illegal option");
    }
  }

  // must contain 4 files
  // store them in reversed order (same as before but just flip the final
  // string)

  return EXIT_SUCCESS;
}
