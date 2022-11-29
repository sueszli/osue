#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
./client [-a optargA | {-b optargB | -o} ] -c [optargC] file...
optargA ... int [-50,300]
optargB ... char [69,71]
optargC ... char[8] (exactly!)
maximum of 8 pos-args
*/

char *program_name;

static int usage() {
  printf("[%s]: Usage error\n", program_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  program_name = argv[0];

  bool a = false, b = false, c = false, o = false;
  long aarg = 0;
  char *barg = NULL, *carg = NULL;

  int file_count = 0;
  char *files[8];

  int oc;
  while ((oc = getopt(argc, argv, "-a:b:oc::")) != -1) {
    switch (oc) {
      case 'a':
        if (a) usage();
        a = true;
        aarg = strtol(optarg, NULL, 10);
        break;
      case 'b':
        if (b) usage();
        if (optarg != NULL) {
          size_t len = strlen(optarg);
          if (len < 69 || len > 71) usage();
        }
        b = true;
        barg = optarg;
        break;
      case 'c':
        if (c) usage();
        if (optarg != NULL && strlen(optarg) != 8) usage();
        c = true;
        carg = optarg;
        break;
      case 'o':
        if (o) usage();
        o = true;
        break;
      case 1:
        if (file_count == 8) return usage();
        files[file_count] = optarg;
        file_count++;
        break;
      default:
        usage();
        break;
    }
  }

  if (!c || file_count == 0) usage();
  if (a + b + c > 1) usage();

  printf("a: %i\nb: %i\nc: %i\no: %i\n\naarg: %li\nbarg: %s\ncarg: %s\n\n", a,
         b, c, o, aarg, barg, carg);
  for (int i = 0; i < file_count; i++) {
    printf("%s\n", files[i]);
  }
  return EXIT_SUCCESS;
}
