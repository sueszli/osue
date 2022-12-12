#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define PROTOCOL "http://"

static void usage(const char *msg) {
  fprintf(
      stderr,
      "%s \nUsage: mirror [-l] [-t timeout] [-m maximum] [-f filetype] url \n",
      msg);
  exit(EXIT_FAILURE);
}

static unsigned long strtoulWrapper(char *str) {
  char *endptr;
  errno = 0;
  unsigned long val = strtoul(str, &endptr, 10);
  if (errno != 0) {
    error("strtoul");
  }
  if (endptr == str) {
    usage("strtoul: no digits in option argument");
  }
  if (*endptr != '\0') {
    usage("strtoul: non-digit characters following argument");
  }

  return val;
}

int main(int argc, char *argv[]) {
  bool lOption = false;
  bool tOption = false;
  bool mOption = false;
  bool fOption = false;

  unsigned long timeout;
  unsigned long maximum;
  char *filetype = NULL;

  int opt = 0;
  while ((opt = getopt(argc, argv, "lt:m:f:")) != -1) {
    switch (opt) {
      // [-l]
      case 'l':
        if (lOption) {
          usage("used same option more than once");
        }
        lOption = true;
        printf("-l\n");
        break;

      // [-t timeout]
      case 't':
        if (tOption) {
          usage("used same option more than once");
        }
        tOption = true;
        if (optarg[0] == '-') {
          usage("missing option argument");
        }
        timeout = strtoulWrapper(optarg);
        printf("-t %ld\n", timeout);
        break;

      // [-m maximum]
      case 'm':
        if (mOption) {
          usage("used same option more than once");
        }
        mOption = true;
        if (optarg[0] == '-') {
          usage("missing option argument");
        }
        maximum = strtoulWrapper(optarg);
        printf("-m %ld\n", maximum);
        break;

      // [-f filetype]
      case 'f':
        if (fOption) {
          usage("used same option more than once");
        }
        fOption = true;
        if (optarg[0] == '-') {
          usage("missing option argument");
        }
        filetype = optarg;
        printf("-f %s\n", filetype);
        break;

      default:
        usage("unknown option");
    }
  }

  if ((argc - optind) != 1) {
    usage("not exactly one url given");
  }
  if (strncmp(argv[optind], PROTOCOL, strlen(PROTOCOL)) != 0) {
    usage("url missing http:// prefix");
  }
  char *url = argv[optind];
  printf("url: %s\n", url);

  exit(EXIT_SUCCESS);
}