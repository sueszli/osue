#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
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

#define usage(msg)                                                          \
  do {                                                                      \
    fprintf(stderr,                                                         \
            "Usage: %s\nSYNOPSIS:\n\t./client -p PORT [-b PORT] [-r|-s]\n", \
            msg);                                                           \
    exit(EXIT_FAILURE);                                                     \
  } while (0)

#define SERVER_IPADDR_STR "127.0.0.1"
#define BUFFER_SIZE (1 << 4)

enum mode_t { mode_unset, mode_request, mode_shutdown };

static uint16_t parse_port_number(char *str) {
  long val;
  char *endptr;

  val = strtol(str, &endptr, 10);
  if (endptr == str || *endptr != '\0') {
    usage("port number not parsable");
  }
  if (val <= 0 || val > 65535) {
    usage("port number out of range");
  }
  return (uint16_t)val;
}

static int getSocketFd(uint16_t connect_port) {
  struct sockaddr_in addr;

  int cfd = socket(AF_INET, SOCK_STREAM, 0);
  if (cfd == -1) {
    perror("socket");
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_port = htons(connect_port);
  addr.sin_family = AF_INET;

  if (!inet_aton(SERVER_IPADDR_STR, (struct in_addr *)&addr.sin_addr.s_addr)) {
    shutdown(cfd, SHUT_RDWR);
    close(cfd);
    perror("inet_aton");
    return -1;
  }

  if (connect(cfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    shutdown(cfd, SHUT_RDWR);
    close(cfd);
    perror("connect");
    return -1;
  }

  return cfd;
}

static void communicate(int sockfd, mode_t mode) {
  // send request message
  msg_t request;
  memset(request, 0, sizeof(request));
  if (mode == mode_request) {
    strcpy(request, "request");
  } else {
    strcpy(request, "shutdown");
  }
  if (write(sockfd, request, strlen(request)) == -1) {
    close(sockfd);
    error("write");
  }

  // receive response message
  if (mode == mode_request) {
    msg_t response;
    memset(response, 0, sizeof(response));
    if (read(sockfd, response, sizeof(response)) == -1) {
      close(sockfd);
      error("read");
    }
    printf("%s\n", response);
  }

  // clean
  close(sockfd);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  uint16_t server_port = 0;
  uint16_t backup_port = 0;
  enum mode_t mode = mode_unset;

  int opt;
  while ((opt = getopt(argc, argv, "p:b:rs")) != -1) {
    switch (opt) {
      case 'p':
        if (server_port > 0) {
          usage("server port option set more than once");
        }
        if (optarg[0] == '-') {
          usage("missing port argument");
        }
        server_port = parse_port_number(optarg);
        break;

      case 'b':
        if (backup_port > 0) {
          usage("backup port option set more than once");
        }
        if (optarg[0] == '-') {
          usage("missing backup port argument");
        }
        backup_port = parse_port_number(optarg);
        break;

      case 'r':
      case 's':
        if (mode != mode_unset) {
          usage("mode set more than once");
        }
        mode = (opt == 'r') ? mode_request : mode_shutdown;
        break;

      default:
        usage("illegal option");
        break;
    }
  }

  if (server_port == 0) {
    usage("missing server port");
  }
  if (mode == mode_unset) {
    usage("missing mode");
  }
  if (optind < argc) {
    usage("illegal positional arguments");
  }

  fprintf(stdout, "-p %d\n", server_port);
  fprintf(stdout, "-b %d\n", backup_port);
  fprintf(stdout, "%s\n", (mode == mode_request ? "-request" : "-shutdown"));

  // try connecting to primary socket
  int psockfd = getSocketFd(server_port);
  if (psockfd != -1) {
    communicate(psockfd, mode);
  }

  // try connecting to backup socket
  int bsockfd = getSocketFd(backup_port);
  if ((backup_port != 0) && (bsockfd != -1)) {
    communicate(bsockfd, mode);
  }

  exit(EXIT_FAILURE);
}
