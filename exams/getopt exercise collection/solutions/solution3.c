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
  if (argc < 6) {
    usage("too few arguments");
  }
  if (argc > 7) {
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
        printf("-a %s\n", optargA);
        break;

      case 'b':
        if (optB) {
          usage("same option occured more than once");
        }
        optB = true;
        if (optA || optC) {
          usage("did not use option a xor b xor c");
        }
        printf("-b\n");
        break;

      case 'c':
        if (optC) {
          usage("same option occured more than once");
        }
        optC = true;
        if (optA || optB) {
          usage("did not use option a xor b xor c");
        }
        // always check argv[optind] before checking optarg -> the order matters
        if ((argv[optind] != NULL) && (argv[optind][0] != '-')) {
          optargC = argv[optind];
        }
        if ((optarg != NULL) && (optarg[0] != '-')) {
          optargC = optarg;
        }
        printf("-c %s\n", optargC);
        break;

      default:
        usage("illegal option");
    }
  }

  if (((argc - optind) != 0) && (optargC != NULL) &&
      (strcmp(optargC, argv[optind]) == 0)) {
    optind++;
  }

  if ((argc - optind) != 4) {
    usage("not exactly 4 positional arguments given");
  }

  char *total = malloc(1 * sizeof(char));
  if (total == NULL) {
    error("malloc");
  }
  total[0] = '\0';

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
  printf("total: %s\n", total);

  char *totalReversed = malloc((strlen(total) + 1) * sizeof(char));
  for (size_t i = 0; i < strlen(total); i++) {
    totalReversed[i] = total[strlen(total) - i - 1];
  }
  totalReversed[strlen(total)] = '\0';
  printf("total reversed: %s\n", totalReversed);

  free(total);
  free(totalReversed);

  return EXIT_SUCCESS;
}
