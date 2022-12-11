#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

char* program_name = NULL;

void usage(const char* message) {
  fprintf(stderr, "%s\n", message);
  fprintf(stderr,
          "Usage: %s [-a optargA] [-e] -c [optargC] [-b optargB [-d] ]\n",
          program_name);
  exit(EXIT_FAILURE);
}

/*
- ./client [-a optargA] [-e] -c [optargC] [-b optargB [-d] ]

- hint: optargC can only be specified by -coptargC (not -c optargC)

- all optargs are char* (for simplicity)
- hardest test-cases:
    - -d specified, although -b not specified (FAIL)
    - -d not specified, -b specified (PASS)
*/
int main(int argc, char** argv) {
  if (argc < 2) {
    usage("too few arguments");
  }
  if (argc > 9) {
    usage("too many arguments");
  }
  program_name = argv[0];

  bool aOption = false;
  bool eOption = false;
  bool cOption = false;
  bool bOption = false;
  bool dOption = false;

  char* optargA = NULL;
  char* optargC = NULL;
  char* optargB = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "a:ec::b:d")) != -1) {
    switch (opt) {
      // [-a optargA]
      case 'a':
        if (aOption) {
          usage("used same option more than once");
        }
        aOption = true;
        if ((optarg == NULL) || (optarg[0] == '-')) {
          usage("missing option argument");
        }
        optargA = optarg;
        printf("-a %s\n", optargA);
        break;

      // [-e]
      case 'e':
        if (eOption) {
          usage("used same option more than once");
        }
        eOption = true;
        printf("-e\n");
        break;

      // -c [optargC]
      // check both optarg and argv[optind] for optargC (they behave differently
      // on different edge cases)
      case 'c':
        if (cOption) {
          usage("used same option more than once");
        }
        cOption = true;
        if ((argv[optind] != NULL) && (argv[optind][0] != '-')) {
          optargC = argv[optind];
        }
        if ((optarg != NULL) && (optarg[0] != '-')) {
          optargC = optarg;
        }
        printf("-c %s\n", optargC);
        break;

      // [-b optargB [-d] ]
      case 'b':
        if (bOption) {
          usage("used same option more than once");
        }
        bOption = true;
        if ((optarg == NULL) || (optarg[0] == '-')) {
          usage("missing option argument");
        }
        optargB = optarg;
        printf("-b %s\n", optargB);
        break;

      // [-b optargB [-d] ]
      case 'd':
        if (dOption) {
          usage("used same option more than once");
        }
        dOption = true;
        if (!bOption) {
          usage("set d option without setting d");
        }
        printf("-d\n");
        break;

      default:
        usage("illegal option");
    }
  }

  if (!cOption) {
    usage("did not set c option");
  }

  if ((argc - optind) > 0) {
    usage("illegal positional argument");
  }

  exit(EXIT_SUCCESS);
}
