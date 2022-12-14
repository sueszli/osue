#include "client.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
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

/***********************************************************************
 * Task 1
 * ------
 * Implement argument parsing for the client. Synopsis:
 *   ./client [-p PORT] {-g|-s VALUE} ID
 *
 * Hints: getopt(3), UINT16_MAX, parse_number (client.h)
 ***********************************************************************/
/*
typedef enum { GET = 0, SET = 1, UNDEF = 2 } cmd_t;

struct args {
  uint16_t portnum;     // < port number [1024;UINT16_MAX]
  const char *portstr;  // < port number as string
  cmd_t cmd;            // < command (GET, SET)
  uint8_t value;        // < set value [0;127]
  uint8_t id;           // < device id [0;63]
};
*/
int main(int argc, char **argv) {
  if (argc > 6) {
    usage();
  }
  if (argc < 3) {
    usage();
  }
  program_name = argv[0];

  struct args arguments = {DEFAULT_PORTNUM, DEFAULT_PORTNUM_STR, UNDEF, 0, 0};

  bool optP = false;
  bool optG = false;
  bool optS = false;

  int opt;
  while ((opt = getopt(argc, argv, "p:gs:")) != -1) {
    switch (opt) {
      case 'p':
        if (optP) {
          usage();
        }
        optP = true;
        if (optarg[0] == '-') {
          usage();
        }
        long port = parse_number(optarg);
        if ((port < 1024) || (port > UINT16_MAX)) {
          usage();
        }
        arguments.portnum = (uint16_t)port;
        arguments.portstr = optarg;
        printf("-p %ld %s\n", port, optarg);
        break;

      case 'g':
        if (optG) {
          usage();
        }
        optG = true;
        arguments.cmd = GET;
        printf("-g\n");
        break;

      case 's':
        if (optS) {
          usage();
        }
        optS = true;
        if (optarg[0] == '-') {
          usage();
        }
        long value = parse_number(optarg);
        if ((value < 0) || (value > 127)) {
          usage();
        }
        arguments.value = (uint8_t)value;
        arguments.cmd = SET;
        printf("-s %ld \n", value);
        break;

      default:
        usage();
        break;
    }
  }

  if (((!optG) && (!optS)) || (optG && optS)) {
    usage();
  }

  if ((argc - optind) != 1) {
    usage();
  }
  char *idStr = argv[optind];
  long id = parse_number(idStr);
  if ((id < 0) || (id > 63)) {
    usage();
  }
  arguments.id = (uint8_t)id;
  printf("id %ld \n", id);

  apply_command(arguments);  // don't remove this line

  exit(EXIT_SUCCESS);
}
