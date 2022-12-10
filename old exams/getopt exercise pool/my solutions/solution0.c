// Note: this exercise is the 'client.c' file from the 2017 exam

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

char *program_name = "<not yet set>";

typedef enum { GET = 0, SET = 1, UNDEF = 2 } cmd_t;

struct args {
  uint16_t portnum;     // port number
  const char *portstr;  // port number as string
  cmd_t cmd;            // command (GET, SET)
  uint8_t value;        // set value
  uint8_t id;           // device id
};

void usage(const char *message) {
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

  error("this is a test");

  struct args arguments = {DEFAULT_PORTNUM, DEFAULT_PORTNUM_STR, UNDEF, 0, 0};

  printf("pn: %d\n", arguments.portnum);
  printf("pns: %s\n", arguments.portstr);
  printf("cmd: %d\n", arguments.cmd);
  printf("value: %d\n", arguments.value);
  printf("id: %d\n", arguments.id);

  return EXIT_SUCCESS;
}
