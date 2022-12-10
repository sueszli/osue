#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
./client [-a optargA] [-e] -c [optargC] [-b optargB [-d]]
*/

char *program_name;

static int usage() {
  printf("[%s]: Usage error\n", program_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  program_name = argv[0];

  bool a = false, b = false, c = false, d = false, e = false;
  int aarg = 0, barg = 0, carg = 0;

  int oc;
  while ((oc = getopt(argc, argv, "a:ec::b:d")) != -1) {
    switch (oc) {
      case 'a':
        if (a) usage();
        a = true;
        aarg = strtol(optarg, NULL, 10);
        break;
      case 'e':
        if (e) usage();
        e = true;
        break;
      case 'c':
        if (c) usage();
        c = true;
        if (optarg != NULL) carg = strtol(optarg, NULL, 10);
        break;
      case 'b':
        if (b) usage();
        b = true;
        barg = strtol(optarg, NULL, 10);
        break;
      case 'd':
        if (d) usage();
        d = true;
        break;
      default:
        usage();
        break;
    }
  }

  if (!c) usage();
  if (!b && d) usage();
  if (optind != argc) usage();

  printf("a: %i\nb: %i\nc: %i\nd: %i\ne: %i\n\naarg: %i\nbarg: %i\ncarg: %i\n",
         a, b, c, d, e, aarg, barg, carg);
  return EXIT_SUCCESS;
}