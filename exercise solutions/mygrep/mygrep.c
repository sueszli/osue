#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define usage(msg)                                                           \
  do {                                                                       \
    fprintf(stderr, "%s\n", msg);                                            \
    fprintf(stderr,                                                          \
            "Usage: ./mygrep mygrep [-i] [-o outfile] keyword [file...]\n"); \
    exit(EXIT_FAILURE);                                                      \
  } while (0)

int main(int argc, char* argv[]) {
  bool optI = false;

  bool optO = false;
  FILE* outputStream = stdout;

  int opt;
  while ((opt = getopt(argc, argv, "io:")) != -1) {
    switch (opt) {
      case 'i':
        if (optI) {
          usage("multiple usage of option");
        }
        optI = true;
        if ((optarg == NULL) || (optarg[0] == '-')) {
          usage("missing argument");
        }
        printf("-i\n");
        break;

      case 'o':
        if (optO) {
          usage("multiple usage of option");
        }
        optO = true;
        if ((optarg == NULL) || (optarg[0] == '-')) {
          usage("missing argument");
        }
        outputStream = fopen(optarg, "r");
        if (outputStream == NULL) {
          error("fopen");
        }
        printf("-o %s\n", optarg);
        break;

      default:
        usage("unknown option");
    }
  }

  if ((argc - optind) == 0) {
    usage("missing keyword");
  }
  char* keyword = argv[optind++];

  // WIP

  do {
    FILE* inputStream;
    if ((argc - optind) == 0) {
      inputStream = stdin;

    } else {
      printf("processing path: %s\n", argv[optind]);
      inputStream = fopen(argv[optind], "r");
      if (inputStream == NULL) {
        error("fopen");
      }
    }

    fprintf(outputStream, "\n");
  } while (++optind < argc);

  exit(EXIT_SUCCESS);
}
