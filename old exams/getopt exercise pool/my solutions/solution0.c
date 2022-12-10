#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define DEFAULT_PORTNUM (2017)
#define DEFAULT_PORTNUM_STR "2017"

char *program_name = NULL;

typedef enum { GET = 0, SET = 1, UNDEF = 2 } cmd_t;

struct args {
  uint16_t portnum;     // port number
  const char *portstr;  // port number as string
  cmd_t cmd;            // command (GET, SET)
  uint8_t value;        // set value
  uint8_t id;           // device id
};

static void usage(const char *message) {
  fprintf(stderr, "%s\n", message);
  fprintf(stderr, "Usage: %s [-p PORT] {-g|-s VALUE} ID\n", program_name);
  exit(EXIT_FAILURE);
}

/***********************************************************************
 * Task 1
 * ------
 * Implement argument parsing for the client. Synopsis:
 *   ./client [-p PORT] {-g|-s VALUE} ID
 *
 * Legal ranges:
 *   PORT:  long [1024;UINT16_MAX]
 *   VALUE: int  [0;127]
 *   ID:    int  [0;63]
 *
 * Call usage() if invalid options or arguments are given (there is no
 * need to print a description of the problem).
 *
 * Hints: getopt(3), UINT16_MAX, parse_number (client.h),
 *        struct args (client.h)
 ***********************************************************************/

int main(int argc, char **argv) {
  if (argc > 0) {
    program_name = argv[0];
  }

  int opt;
  while ((opt = getopt(argc, argv, "p:gs:")) != -1) {
    switch (opt) {
      case 'p':
        if (optarg[0] == '-') {  // else may use -g as arg
          usage("no argument for p flag");
        }
        printf("set flag -p with: %s\n", optarg);
        break;

      case 'g':
        printf("set flag -g");
        break;

      case 's':
        printf("set flag -s with: %s\n", optarg);
        break;

      default:
        usage("illegal argument");
    }
  }

  for (int i = optind; i < argc; i++) {
    printf("non option: %s\n", argv[i]);
  }

  // optional: p with argument

  // required: g flag / s flag with argument

  // required: 1 non-option-argument

  // ...

  struct args arguments = {DEFAULT_PORTNUM, DEFAULT_PORTNUM_STR, UNDEF, 0, 0};

  // store input in struct

  // printf("pn: %d\n", arguments.portnum);
  // printf("pns: %s\n", arguments.portstr);
  // printf("cmd: %d\n", arguments.cmd);
  // printf("value: %d\n", arguments.value);
  // printf("id: %d\n", arguments.id);

  return EXIT_SUCCESS;
}
