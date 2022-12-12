#include <assert.h>
#include <ctype.h>
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

char *program_name = NULL;

static void usage(const char *message) {
  fprintf(stderr, "%s\n", message);
  fprintf(stderr,
          "Usage: %s [-a optargA | -b optargB | -o ] -c [optargC] file... "
          "(max. 8 files)\n",
          program_name);
  exit(EXIT_FAILURE);
}

static long strtolWrapper(char *str) {
  char *endptr;
  errno = 0;
  long val = strtol(str, &endptr, 10);
  if (errno != 0) {
    perror("strtol");
    exit(EXIT_FAILURE);
  }
  if (endptr == str) {
    usage("strtol -> no digits were found\n");
  }
  if (*endptr != '\0') {
    usage("strtol -> not only decimal were digits used");
  }
  return val;
}

/*
  - ./client [-a optargA | -b optargB | -o ] -c [optargC] file... (max. 8)
  - hint: optargC can only be specified by -coptargC (not -c optargC)
  - optargA ... int [-50,300]
  - optargB ... char
  - optargC ... char[8] (exactly!)
  - maximum of 8 pos-args, no minimum, cat all to one total_string
*/
int main(int argc, char **argv) {
  if (argc > 13) {
    usage("too many arguments");
  }
  if (argc < 2) {
    usage("too few arguments");
  }
  program_name = argv[0];

  bool aOption = false;
  bool bOption = false;
  bool oOption = false;
  bool cOption = false;

  int optargA = 0;
  char optargB = 0;
  char *optargC = NULL;

  int opt = 0;
  while ((opt = getopt(argc, argv, "a:b:oc::")) != -1) {
    switch (opt) {
      // [-a optargA | -b optargB | -o ]
      // optargA range: int [-50,300]
      case 'a':
        if (aOption) {
          usage("used option more than once");
        }
        aOption = true;
        if (bOption || oOption) {
          usage("did not use a xor b xor o option");
        }
        if ((strlen(optarg) > 1) && (optarg[0] == '-') &&
            (!isdigit(optarg[1]))) {  // allow negative numbers
          usage("no argument for option");
        }
        long aVal = strtolWrapper(optarg);
        if ((aVal < -50) || (aVal > 300)) {
          usage("argument not in range");
        }
        optargA = (int)aVal;
        printf("-a %d\n", optargA);
        break;

      // [-a optargA | -b optargB | -o ]
      // optargB range: char
      case 'b':
        if (bOption) {
          usage("used option more than once");
        }
        bOption = true;
        if (aOption || oOption) {
          usage("did not use a xor b xor o option");
        }
        if (optarg[0] == '-') {
          usage("no argument for option");
        }
        if (strlen(optarg) > 1) {
          usage("argument for option b is too long");
        }
        optargB = optarg[0];
        printf("-b %c\n", optargB);
        break;

      // [-a optargA | -b optargB | -o ]
      case 'o':
        if (oOption) {
          usage("used option more than once");
        }
        oOption = true;
        if (aOption || bOption) {
          usage("did not use a xor b xor o option");
        }
        printf("-o\n");
        break;

      // -c [optargC]
      // optargC range: char[8]
      case 'c':
        if (cOption) {
          usage("used option more than once");
        }
        cOption = true;
        if ((argv[optind] != NULL) && (argv[optind][0] != '-')) {
          optargC = argv[optind];
        }
        if ((optarg != NULL) && (optarg[0] != '-')) {
          optargC = optarg;
        }
        if ((optargC != NULL) && (strlen(optargC) > 7)) {
          usage("argument of option c is too large");
        }
        printf("-c %s\n", optargC);
        break;

      default:
        usage("illegal option");
    }
  }

  if (!cOption) {
    usage("did not use c option");
  }

  // file... (max. 8) -> all concatenated to 'total'
  if ((optargC != NULL) && (argc - optind != 0) &&
      (strcmp(optargC, argv[optind]) == 0)) {
    optind++;
  }

  if ((argc - optind) > 8) {
    usage("too many file paths");
  }

  char *total = malloc(1 * sizeof(char));
  total[0] = '\0';
  if (total == NULL) {
    error("malloc");
  }

  size_t counter = 0;
  for (int i = optind; i < argc; i++) {
    char *elem = argv[i];

    char *newTotal =
        realloc(total, (counter + strlen(elem) + 1) * sizeof(char));
    if (newTotal == NULL) {
      error("realloc");
    }
    total = newTotal;

    for (size_t j = 0; j <= strlen(elem); j++) {
      total[counter++] = elem[j];
    }
    counter--;
  }

  printf("all files: %s\n", total);
  free(total);

  return EXIT_SUCCESS;
}
