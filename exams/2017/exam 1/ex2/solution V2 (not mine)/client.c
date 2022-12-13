#include "client.h"

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

/** name of the executable (for printing messages) */
char *program_name = "client";

/** program entry point */
int main(int argc, char **argv) {
  struct args arguments;

  /* parse program arguments and fill 'arguments' [given] */
  parse_arguments(argc, argv, &arguments);

  /*******************************************************************
   * Task 1
   * ------
   * Connect to server.
   *
   * - Resolve host address, set port. Macros exist (SERVER_IPADDR_STR) and
   *   variables exist (arguments).
   * - Create socket. Use variable 'sockfd' for creation. Socket type
   *   is SOCK_STREAM.
   * - Connect.
   *
   * See also: getaddrinfo(3), socket(2), connect(2), ip(7),
   * error_exit (common.h)
   *******************************************************************/

  /* file descriptor of socket */
  int sockfd;

  /* REPLACE FOLLOWING LINE WITH YOUR SOLUTION */
  // task_1_demo(&sockfd, &arguments);
  struct addrinfo *ai = NULL;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int res = getaddrinfo(NULL, arguments.portstr, &hints, &ai);
  if (res < 0) error_exit(program_name);

  sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
  if (sockfd < 0) error_exit(program_name);

  if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0)
    error_exit(program_name);

  /*******************************************************************
   * Task 2
   * ------
   * Pack and send command to server and receive acknowledge.
   *
   * - Pack the command from the arguments into a buffer.
   * - Send the buffer to the server.
   * - Receive response from the server.
   *   Save response to variables 'nok' and 'value' accordingly.
   *
   * See also: send(2), recv(2)
   *******************************************************************/

  uint8_t nok;
  uint8_t value;

  /* REPLACE FOLLOWING LINE WITH YOUR SOLUTION */
  task_2_demo(&sockfd, &arguments, &nok, &value);

  /* DO NOT CHANGE THE FOLLOWING LINES */
  /* print server response */
  puts((nok) ? "NOK" : "OK");

  if (arguments.cmd == GET && !nok) printf("%d\n", value);

  /* cleanup: close socket */
  close(sockfd);

  exit(EXIT_SUCCESS);
}
