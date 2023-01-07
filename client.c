#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define usage(msg)                                                           \
  do {                                                                       \
    fprintf(stderr,                                                          \
            "Wrong usage: %s\nSYNOPSIS:\n\tclient [-p PORT] [ -o FILE | -d " \
            "DIR ] URL\nEXAMPLE\n\tclient http://www.nonhttps.com/",         \
            msg);                                                            \
    exit(EXIT_FAILURE);                                                      \
  } while (0);

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0);

int main(int argc, char* argv[]) {
  bool optP = false;
  bool optO = false;
  bool optD = false;

  char* port = "80";
  char* outputFile = NULL;
  char* outputDirectory = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "p:o:d:")) != -1) {
    switch (opt) {
      case 'p':
        if (optP) {
          usage("option p used multiple times");
        }
        optP = true;
        if (optarg[0] == '-') {
          usage("missing option p argument");
        }
        port = optarg;
        printf("-p %s\n", port);
        break;

      case 'o':
        if (optO) {
          usage("option o used multiple times");
        }
        if (optD) {
          usage("didn't use option o xor d");
        }
        optO = true;
        if (optarg[0] == '-') {
          usage("missing option o argument");
        }
        outputFile = optarg;
        printf("-o %s\n", outputFile);
        break;

      case 'd':
        if (optD) {
          usage("option d used multiple times");
        }
        if (optO) {
          usage("didn't use option o xor d");
        }
        optD = true;
        if (optarg[0] == '-') {
          usage("missing option d argument");
        }
        outputDirectory = optarg;
        printf("-d %s\n", outputDirectory);
        break;

      default:
        usage("illegal option");
    }
  }

  // check positional argument

  exit(EXIT_SUCCESS);
}