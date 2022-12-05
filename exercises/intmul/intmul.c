#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (true);

#define usage(msg)                                                          \
  do {                                                                      \
    fprintf(                                                                \
        stderr,                                                             \
        "Invalid input: %s.\nEnter 2 hexadecimal numbers to multiply with " \
        "eachother.\nSYNOPSIS:\n\t./intmul {hexNum1} {hexNum2}",            \
        msg);                                                               \
    exit(EXIT_FAILURE);                                                     \
  } while (true);

void validateInput(int argc, char* argv[]) {
  if (argc < 3) {
    usage("too few arguments");
  }

  char* hexNum1 = argv[1];
  char* hexNum2 = argv[2];
  printf("%s\n%s\n", hexNum1, hexNum2);
}

int main(int argc, char* argv[]) {
  validateInput(argc, argv);

  return EXIT_SUCCESS;
}