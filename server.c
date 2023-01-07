#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define usage(msg)                                                 \
  do {                                                             \
    fprintf(stderr, "Wrong usage: %s\nSYNOPSIS SERVER: ...", msg); \
    exit(EXIT_FAILURE);                                            \
  } while (0);

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0);

int main(int argc, char* argv[]) { exit(EXIT_SUCCESS); }