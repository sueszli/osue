// Client either sends a status request or a shutdown command to a server.
// If communication with the main server fails, the client tries to send the
// message to the backup server.

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

#define SERVER_IPADDR_STR "127.0.0.1"

enum mode_t { mode_unset, mode_request, mode_shutdown };

static void usage(void) {
  (void)fprintf(stderr, "Usage: ./client -p PORT [-b PORT] [-r|-s]\n");
  exit(EXIT_FAILURE);
}

static uint16_t parse_port_number(char *str) {
  long val;
  char *endptr;

  val = strtol(str, &endptr, 10);
  if (endptr == str || *endptr != '\0') {
    return 0;
  }
  if (val <= 0 || val > 65535) {
    return 0;
  }
  return (uint16_t)val;
}

int main(int argc, char **argv) {
  enum mode_t mode =
      mode_unset; /* whether client sends status request or shutdown */
  uint16_t server_port = 0; /* port of main server */
  uint16_t backup_port = 0; /* port of backup server */
  msg_t message;            /* client message */

  int getopt_result;
  while ((getopt_result = getopt(argc, argv, "p:b:rs")) != -1) {
    switch (getopt_result) {
      case 'r':
      case 's':
        if (mode != mode_unset) {
          usage();
        }
        mode = (getopt_result == 'r') ? mode_request : mode_shutdown;
        break;
      case 'p':
        if (server_port > 0) {
          usage();
        }
        if ((server_port = parse_port_number(optarg)) == 0) {
          (void)fprintf(stderr, "Bad port number: %s\n", optarg);
          usage();
        }
        break;
      case 'b':
        if (backup_port > 0) {
          usage();
        }
        if ((backup_port = parse_port_number(optarg)) == 0) {
          (void)fprintf(stderr, "Bad port number: %s\n", optarg);
          usage();
        }
        break;
      case '?':
        usage();
        break;
      default:
        assert(0);
    }
  }
  /********************************/
  /* TODO: insert your code below */
  /********************************/

  /* Check that there are no additional (superfluous) arguments and
     that all required options (-r or -s, and -p) are present. */
  if (optind < argc) {
    usage();
  }

  if (server_port == 0) {
    usage();
  }

  if (mode == mode_unset) {
    usage();
  }

  /* Create socket */

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    fprintf(stderr, "socket error");
    exit(EXIT_FAILURE);
  }

  /* Resolve host / convert IP String */

  struct sockaddr_in serv;

  serv.sin_port = htons(server_port);
  serv.sin_family = AF_INET;

  if (inet_pton(AF_INET, SERVER_IPADDR_STR, &serv.sin_addr) != 1) {
    fprintf(stderr, "inet_pton failed");
    exit(EXIT_FAILURE);
  }

  /* Connect to server on server_port */
  int error = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));

  /* On error, try to connect to backup_port (if specified) */

  if (error < 0 && backup_port > 0) {
    serv.sin_port = htons(backup_port);
    error = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));
  }

  if (error < 0) {
    fprintf(stderr, "connect failed");
    exit(EXIT_FAILURE);
  }

  /* prepare message, and send it to the server */
  (void)memset(message, 0x0, sizeof(msg_t));

  // char msg[];
  if (mode == mode_request) {
    strcpy(message, "request");
  } else {
    strcpy(message, "shutdown");
  }

  if (write(sockfd, message, sizeof(message)) < 0) {
    fprintf(stderr, "write error");
    exit(EXIT_FAILURE);
  }

  /* In mode -r, receive and print reply (followed by newline) */

  if (mode == mode_request) {
    msg_t response;
    if (read(sockfd, response, sizeof(msg_t)) < 0) {
      fprintf(stderr, "read error");
      exit(EXIT_FAILURE);
    }

    printf(response);
  }

  return EXIT_SUCCESS;
}
