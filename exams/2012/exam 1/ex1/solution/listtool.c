#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

static const char *program_name = NULL;

static void usage(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  fprintf(stderr, "SYNOPSIS: %s [-s | -a NUM] <string>\n", program_name);
  fprintf(stderr,
          "\t-s and -a are only allowed once\n"
          "\tone of them has to be given\n"
          "\t<string> is mandatory\n"
          "\t-s iterates over the list and searches the string <string> in the "
          "list\n"
          "\t-a inserts the string <string> *after* the NUM-th element of the "
          "list\n"
          "\tif NUM is larger than the number of list entries - 1, append the "
          "entry after the last element\n"
          "\tfor debugging, the whole list can be printed with 'print_list' "
          "(cf., list.h)\n");

  fprintf(stderr,
          "EXAMPLES: (assume the list consists of the entries 'osue', 'test', "
          "'ss2012')\n");
  fprintf(stderr, "\t%s => print usage\n", program_name);
  fprintf(stderr, "\t%s -s -s foo => print usage\n", program_name);
  fprintf(stderr, "\t%s foo => print usage\n", program_name);
  fprintf(stderr, "\t%s -s foo => no,no,no,\n", program_name);
  fprintf(stderr, "\t%s -s ss2012 => no,no,yes,\n", program_name);
  fprintf(stderr, "\t%s -a 0 foo => list becomes: osue, foo, test, ss2012\n",
          program_name);
  fprintf(stderr, "\t%s -a 2 foo => list becomes: osue, test, ss2012, foo\n",
          program_name);
  fprintf(stderr, "\t%s -a 23 foo => list becomes: osue, test, ss2012, foo\n",
          program_name);

  exit(EXIT_FAILURE);
}

static void insert_after(struct listelem *after, const char *const value) {
  /* TODO: insert your code */
  /* when setting 'val' of the list, use strdup(value); */
  /* if you do not use strdup(), destroy() will fail/crash */
  struct listelem *next = malloc(sizeof(struct listelem));
  next->val = strdup(value);
  next->next = after->next;
  after->next = next;
  return;
}

// alternative synopsis: ./listtool {-s <optstr> | -a <num> <optstr>}
int main(int argc, char **argv) {
  if (argc < 3) {
    usage("too few arguments");
  }
  if (argc > 4) {
    usage("too many arguments");
  }
  program_name = argv[0];

  bool optS = false;
  bool optA = false;

  char *optstr = NULL;
  long num = -1;

  int opt;
  while ((opt = getopt(argc, argv, "s:a:")) != -1) {
    switch (opt) {
      case 's':
        if (optS) {
          usage("used option more than once");
        }
        optS = true;
        if (optarg[0] == '-') {
          usage("missing option argument");
        }
        optstr = optarg;
        printf("-s %s", optstr);
        break;

      case 'a':
        if (optA) {
          usage("used option more than once");
        }
        optA = true;
        if (optarg[0] == '-') {
          usage("missing option argument");
        }
        // parse index
        char *endptr;
        long val = strtol(optarg, &endptr, 10);
        if (errno != 0) {
          usage("argument for option a does not contain only digits");
        }
        if (endptr == optarg) {
          usage("missing option argument");
        }
        if (*endptr != '\0') {
          usage("argument for option a does not contain only digits in suffix");
        }
        num = val;
        break;

      default:
        usage("illegal option");
        break;
    }
  }

  if ((optS && optA) || ((!optS) && (!optA))) {
    usage("did not use option s xor a");
  }

  if (optA) {
    if ((argc - optind) != 1) {
      usage("no positional argument for option a");
    }
    optstr = argv[optind];
  }

  /* do not touch */
  struct listelem *head;
  struct listelem *current;

  head = init_list("head");
  populate_list(head);
  current = head;
  /* end do not touch */

  /* SEARCH:
   * iterate over the whole list and find the given <string>
   * remember: the 'next' pointer of the last list entry is set to NULL.
   * for every element print:
   * 'yes,' if it is equal to <string>, else 'no,'
   */
  if (opt_s != -1) {
    while (current != NULL) {
      if (strcmp(current->val, optstr) == 0) {
        printf("yes,");
      } else {
        printf("no,");
      }
      current = current->next;
    }
    printf("\n"); /* do not remove this line */
  }

  /* ADD:
   * insert the string <string> *after* the NUM-th element of the list
   * if NUM is larger than the number of list entries - 1, append the entry
   * after the last element. for debugging, the list can be printed with
   * 'print_list(head)'
   */
  if (opt_a != -1) {
    /* iterate over the list and stop at the right entry */
    (int i = 0; i < num && current->next != NULL; i++) { /* TODO: change it */
      currefornt = current->next;
    }
    print_list(head);
    insert_after(current, optstr);  // assuming we stopped at current
    print_list(head);
    check_list(head);
  }

  destroy_list(head);
  exit(EXIT_SUCCESS);
}
