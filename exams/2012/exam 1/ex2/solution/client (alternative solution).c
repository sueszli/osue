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

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define SERVER_IPADDR_STR "127.0.0.1"

static void usage(void) {
  fprintf(stderr, "Usage: ./client -p PORT [-b PORT] [-r|-s]\n");
  exit(EXIT_FAILURE);
}

enum mode_t { mode_unset, mode_request, mode_shutdown };

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
  enum mode_t mode = mode_unset;
  uint16_t server_port = 0;
  uint16_t backup_port = 0;
  msg_t message;

  int getopt_result;
  while ((getopt_result = getopt(argc, argv, "p:b:rs")) != -1) {
    switch (getopt_result) {
      case 'p':
        if (server_port > 0) {
          usage();
        }
        if ((server_port = parse_port_number(optarg)) == 0) {
          fprintf(stderr, "Bad port number: %s\n", optarg);
          usage();
        }
        break;

      case 'b':
        if (backup_port > 0) {
          usage();
        }
        if ((backup_port = parse_port_number(optarg)) == 0) {
          fprintf(stderr, "Bad port number: %s\n", optarg);
          usage();
        }
        break;

      case 'r':
      case 's':
        if (mode != mode_unset) {
          fprintf(stderr, "Illegal option\n");
          usage();
        }
        mode = (getopt_result == 'r') ? mode_request : mode_shutdown;
        break;

      default:
        fprintf(stderr, "Illegal option\n");
        usage();
    }
  }

  if (optind < argc) {
    usage();
  }
  if (server_port == 0) {
    usage();
  }
  if (mode == mode_unset) {
    usage();
  }

  // create socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "socket error");
    exit(EXIT_FAILURE);
  }

  // resolve host / convert IP string to binary form
  struct sockaddr_in serv;

  serv.sin_port = htons(server_port);
  serv.sin_family = AF_INET;

  if (inet_pton(AF_INET, SERVER_IPADDR_STR, &serv.sin_addr) != 1) {
    fprintf(stderr, "inet_pton failed");
    exit(EXIT_FAILURE);
  }

  // connect to server
  int error = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));

  // try to connect to backup_port (if specified)
  if (error < 0 && backup_port > 0) {
    serv.sin_port = htons(backup_port);
    error = connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));
  }
  if (error < 0) {
    fprintf(stderr, "connect failed");
    exit(EXIT_FAILURE);
  }

  // send request
  memset(message, 0x0, sizeof(msg_t));

  if (mode == mode_request) {
    strcpy(message, "request");
  } else {
    strcpy(message, "shutdown");
  }

  if (write(sockfd, message, sizeof(message)) == -1) {
    fprintf(stderr, "write error");
    exit(EXIT_FAILURE);
  }

  // print response
  if (mode == mode_request) {
    msg_t response;
    if (read(sockfd, response, sizeof(msg_t)) == -1) {
      fprintf(stderr, "read error");
      exit(EXIT_FAILURE);
    }

    printf(response);
  }

  return EXIT_SUCCESS;
}
