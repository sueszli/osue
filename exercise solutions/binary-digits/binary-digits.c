#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define usage(msg)                                                            \
  do {                                                                        \
    fprintf(stderr, "%s\n", msg);                                             \
    fprintf(stderr,                                                           \
            "Usage: ./binary-digits [-d DELAY] [-o OUTPUTFILE] [FILE]...\n"); \
    exit(EXIT_FAILURE);                                                       \
  } while (0)

int main(int argc, char* argv[]) {
  bool optD = false;
  double delay;

  bool optO = false;
  FILE* outputStream = stdout;

  int opt;
  while ((opt = getopt(argc, argv, "d:o:")) != -1) {
    switch (opt) {
      case 'd':
        if (optD) {
          usage("multiple usage of option");
        }
        optD = true;
        if ((optarg == NULL) || (optarg[0] == '-')) {
          usage("missing argument");
        }
        errno = 0;
        char* endptr;
        delay = strtod(optarg, &endptr);
        if (errno != 0) {
          error("strtod");
        }
        if (endptr == optarg) {
          usage("empty argument");
        }
        if (*endptr != '\0') {
          usage("non digit suffix in argument");
        }
        printf("-d %f\n", delay);
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

  const bool chooseStdIn = (argc - optind) == 0;

  do {
    FILE* inputStream;
    if (chooseStdIn) {
      inputStream = stdin;

    } else {
      printf("processing path: %s\n", argv[optind]);
      inputStream = fopen(argv[optind], "r");
      if (inputStream == NULL) {
        error("fopen");
      }
    }

    int in;
    while ((in = fgetc(inputStream)) != EOF) {
      for (int i = 7; i >= 0; i--) {
        if (in & (1 << i)) {  // get most significant bit
          fputc('1', outputStream);
        } else {
          fputc('0', outputStream);
        }
      }
    }
    fprintf(outputStream, "\n");
  } while (++optind < argc);

  exit(EXIT_SUCCESS);
}
