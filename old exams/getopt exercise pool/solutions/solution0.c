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

static unsigned long strtoulWrapper(const char *str) {
  char *endptr;
  errno = 0;
  unsigned long val = strtoul(str, &endptr, 10);
  if (errno != 0) {
    error("strtoul");
  }
  if (endptr == str) {
    usage("empty option argument");
  }
  if (*endptr != '\0') {
    usage("illegal option argument");
  }
  return val;
}

/***********************************************************************
 * Task 1
 * ------
 * Implement argument parsing for the client. Synopsis:
 *   ./client [-p PORT] {-g|-s VALUE} ID
 *
 * Call usage() if invalid options or arguments are given (there is no
 * need to print a description of the problem).
 *
 * Hints: getopt(3), UINT16_MAX, parse_number (client.h),
 *        struct args (client.h)
 ***********************************************************************/
int main(int argc, char **argv) {
  // check total count
  if (argc > 6) {
    usage("too many options/arguments");
  }

  program_name = argv[0];
  struct args arguments = {DEFAULT_PORTNUM, DEFAULT_PORTNUM_STR, UNDEF, 0, 0};

  bool pOption = false;
  bool gOption = false;
  bool sOption = false;

  // check if options only used once
  // check range
  int opt;
  while ((opt = getopt(argc, argv, "p:gs:")) != -1) {
    switch (opt) {
      case 'p':
        if (optarg[0] == '-') {
          usage("no p option argument used");
        }
        if (pOption) {
          usage("used p option more than once");
        }
        unsigned long port = strtoulWrapper(optarg);
        if (port > UINT16_MAX) {
          usage("p option argument not in uint16_t range");
        }
        pOption = true;
        arguments.portnum = (uint16_t)port;
        arguments.portstr = optarg;
        break;

      case 'g':
        if (gOption) {
          usage("used g option more than once");
        }
        gOption = true;
        arguments.cmd = GET;
        break;

      case 's':
        if (optarg[0] == '-') {
          usage("no s option argument used");
        }
        if (sOption) {
          usage("used s option more than once");
        }
        unsigned long value = strtoulWrapper(optarg);
        if (value > UINT8_MAX) {
          usage("s option argument not in uint8_t range");
        }
        sOption = true;
        arguments.cmd = SET;
        arguments.value = (uint8_t)value;
        break;

      default:
        usage("illegal option");
    }
  }

  // check if required arguments exist
  if (!pOption) {
    usage("no p option used");
  }
  if ((gOption && sOption) || (!gOption && !sOption)) {
    usage("not g xor s option used");
  }

  // get positional arg
  if ((argc - optind) != 1) {
    usage("not exactly one ID argument");
  }
  unsigned long id = strtoulWrapper(argv[optind]);
  if (id > UINT8_MAX) {
    usage("ID argument in illegal range");
  }
  arguments.id = (uint8_t)id;

  // print
  printf("pn: %d\n", arguments.portnum);
  printf("pns: %s\n", arguments.portstr);
  printf("cmd: %d\n", arguments.cmd);
  printf("value: %d\n", arguments.value);
  printf("id: %d\n", arguments.id);

  return EXIT_SUCCESS;
}
