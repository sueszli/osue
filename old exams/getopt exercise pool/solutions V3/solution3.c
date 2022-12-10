#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** name of the executable (for printing messages) */
char *program_name = "<not yet set>";

void usage(const char *message);

int main(int argc, char **argv) {
  /* set program_name */
  if (argc > 0) {
    program_name = argv[0];
  }

  /* COMPLETE AND EXTEND THE FOLLOWING CODE */
  if (argc < 6) usage("Too few args.");
  if (argc > 7) usage("Too many args.");

  // ./client {-a optargA | -b | -c [optargC] } file... (exactly 4 files)

  char *a = NULL;
  char *c = NULL;

  char *endptr;

  bool b_set = false;
  bool abc_occured = false;

  int posArgCount = 0;

  char *first;
  char *second;
  char *third;
  char *fourth;

  char c1;
  while ((c1 = getopt(argc, argv, "-ba:c::")) != -1) {
    switch (c1) {
      case 'a':
        if (abc_occured) usage("Specify exactly one of a/b/c as option.");
        abc_occured = true;
        c = optarg;
        if (optarg == endptr) usage("optargA not parsable");
        break;
      case 'b':
        if (abc_occured) usage("Specify exactly one of a/b/c as option.");
        abc_occured = true;
        b_set = true;
        break;
      case 'c':
        if (abc_occured) usage("Specify exactly one of a/b/c as option.");
        abc_occured = true;
        c = optarg;
        break;
      case 1:
        posArgCount++;
        if (posArgCount > 4) usage("Specify exactly 4 positional arguments.");

        switch (posArgCount) {
          case 1:
            first = optarg;
            break;
          case 2:
            second = optarg;
            break;
          case 3:
            third = optarg;
            break;
          case 4:
            fourth = optarg;
            break;
          default:
            usage("This line should not be printed!");
        }
        break;
      default:
        usage("(getopt detected the error)");
    }
  }

  if (!abc_occured) usage("Specify exactly one of a/b/c as option.");
  if (posArgCount != 4) usage("If this line gets printed, then I'm retarted.");

  char total_string[2048];
  strcpy(total_string, fourth);
  strcat(total_string, third);
  strcat(total_string, second);
  strcat(total_string, first);

  printf("a: %s\n", a);
  printf("b_set: %s\n", b_set ? "true" : "false");
  printf("c: %s\n", c);
  printf("total_string: %s\n", total_string);

  return 0;
}

/** Prints the usage message and exits with EXIT_FAILURE. */
void usage(const char *message) {
  fprintf(stderr, "%s\n", message);
  fprintf(stderr,
          "Usage: %s { -a optargA | -b | -c [optargC] } file... (exactly 4 "
          "files)\n",
          program_name);
  exit(EXIT_FAILURE);
}
