#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

static void wait(double totalTime) {
  time_t seconds = (time_t)totalTime;  // truncate
  long nanoseconds = (long)((totalTime - (double)seconds) * 1e9);
  struct timespec config = {seconds, nanoseconds};
  if (nanosleep(&config, NULL) == -1) {
    error("nanosleep");
  }
}

int main(int argc, char* argv[]) {
  bool optD = false;
  double sleepTime;

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
        sleepTime = strtod(optarg, &endptr);
        if (errno != 0) {
          error("strtod");
        }
        if (endptr == optarg) {
          usage("empty argument");
        }
        if (*endptr != '\0') {
          usage("non digit suffix in argument");
        }
        fprintf(stderr, "-d %f\n", sleepTime);
        break;

      case 'o':
        if (optO) {
          usage("multiple usage of option");
        }
        optO = true;
        if ((optarg == NULL) || (optarg[0] == '-')) {
          usage("missing argument");
        }
        outputStream = fopen(optarg, "w+");
        if (outputStream == NULL) {
          error("fopen");
        }
        fprintf(stderr, "-o %s\n", optarg);
        break;

      default:
        usage("unknown option");
    }
  }

  do {
    FILE* inputStream;
    if ((argc - optind) == 0) {
      inputStream = stdin;
    } else {
      fprintf(stderr, "processing path: %s\n", argv[optind]);
      inputStream = fopen(argv[optind], "r");
      if (inputStream == NULL) {
        error("fopen");
      }
    }

    int byte;
    while ((byte = fgetc(inputStream)) != EOF) {
      // mask most significant bit with 1
      for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i)) {
          fputc('1', outputStream);
        } else {
          fputc('0', outputStream);
        }

        if (optD) {
          fflush(outputStream);
          wait(sleepTime);
        }
      }
    }
    fprintf(outputStream, "\n");

    if (fclose(inputStream) == EOF) {
      error("fclose");
    }
  } while (++optind < argc);

  if (fclose(outputStream) == EOF) {
    error("fclose");
  }
  exit(EXIT_SUCCESS);
}
