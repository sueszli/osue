#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
./client {-a optargA | -b | -c [optargC] } file... (exactly 4 files)
alle optargs sind ints
*/

char *program_name;

static int usage() {
  printf("[%s]: Usage error\n", program_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  program_name = argv[0];

  bool a = false, b = false, c = false;
  long aarg = 0, carg = 0;

  int file_count = 0;
  char *files[4];

  int oc;
  while ((oc = getopt(argc, argv, "-a:bc::")) != -1) {
    switch (oc) {
      case 'a':
        if (a) usage();
        a = true;
        aarg = strtol(optarg, NULL, 10);
        break;
      case 'b':
        if (b) usage();
        b = true;
        break;
      case 'c':
        if (c) usage();
        c = true;
        if (optarg != NULL) carg = strtol(optarg, NULL, 10);
        break;
      case 1:
        if (file_count == 4) usage();
        files[file_count] = optarg;
        file_count++;
        break;
      default:
        usage();
        break;
    }
  }

  if (a + b + c != 1 || file_count != 4) return usage();

  printf("a: %i\nb: %i\nc: %i\n\naarg: %li\ncarg: %li\n\n", a, b, c, aarg,
         carg);
  for (int i = 0; i < file_count; i++) {
    printf("%s\n", files[i]);
  }
}