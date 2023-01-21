#include "client.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

char *program_name = (char *)"client";

static int connectSocket(int connect_port, const char *address) {
  struct sockaddr_in addr;
  int cfd;

  cfd = socket(AF_INET, SOCK_STREAM, 0);
  if (cfd == -1) {
    error_exit("socket");
  }

  if ((connect_port < 0) || (connect_port > USHRT_MAX)) {
    error_exit("illegal socket");
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_port = htons(connect_port);
  addr.sin_family = AF_INET;

  if (!inet_aton(address, (struct in_addr *)&addr.sin_addr.s_addr)) {
    close(cfd);
    error_exit("inet_aton");
  }

  if (connect(cfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    shutdown(cfd, SHUT_RDWR);
    close(cfd);
    error_exit("connect");
  }

  return cfd;
}

static void printArguments(struct args arguments) {
  printf("parsed arguments:\n");
  printf("\tcmd: %d\n", arguments.cmd & 1);
  printf("\tid: ");
  for (int i = 7; i >= 0; i--) {
    printf("%d", (arguments.id >> i) & 1);
  }
  printf("\n");
  printf("\tvalue: ");
  for (int i = 7; i >= 0; i--) {
    printf("%d", (arguments.value >> i) & 1);
  }
  printf("\n");
}

static void print8bits(const char *name, uint8_t bits) {
  printf("%s: ", name);
  for (int i = 7; i >= 0; i--) {
    printf("%d", (bits >> i) & 1);
  }
  printf("\n");
}

static void print16bits(const char *name, uint16_t bits) {
  printf("%s: ", name);
  for (int i = 15; i >= 0; i--) {
    if (i == 7) {
      printf(" ");
    }
    printf("%d", (bits >> i) & 1);
  }
  printf("\n");
}

int main(int argc, char **argv) {
  struct args arguments;
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

  int sockfd = connectSocket(DEFAULT_PORTNUM, SERVER_IPADDR_STR);

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

  // uint8_t value = 0x0;
  // uint8_t nok = 0x0;
  // task_2_demo(&sockfd, &arguments, &nok, &value);

  /*
  "10"
    [server] Received 0x31 0x30.  --> 00110001 - 00110000
    [server] Device ID 12, SET 48
    [server] Device: <unknown>
    SET: 01
    ID: 001100
    value: 110000

  "ab"
    [server] Received 0x61 0x62.  --> 01100001 - 01100010
    [server] Device ID 24, SET 98
    [server] Device: LIGHT 99
    SET: 01
    ID: 011000
    value: 1100010
  */

  // uint16_t r = 0x3130; // server got: 0x30 0x31
  // write(sockfd, &r, 2);

  // print parsed arguments
  // printArguments(arguments);

  // create request
  // 0x72 = 011100 10
  union {
    struct {
      uint8_t cmd : 2;
      uint8_t id : 6;
    } fields;
    uint8_t all;
  } fst;
  fst.fields.cmd = arguments.cmd;
  fst.fields.id = arguments.id;
  printf("first byte: 0x%x\n", fst.all);

  union {
    struct {
      uint8_t value : 7;
      uint8_t : 1;
    } fields;
    uint8_t all;
  } snd;
  snd.fields.value = arguments.value;
  printf("second byte: 0x%x\n", snd.all);

  union {
    struct {
      uint8_t fst : 8;
      uint8_t snd : 8;
    } fields;
    uint16_t all;
  } request;
  request.fields.fst = fst.all;
  request.fields.snd = snd.all;

  // send request to server
  assert(sizeof(request.all) == REQUEST_SIZE);
  if (write(sockfd, &request.all, sizeof(request.all)) == -1) {
    error_exit("write");
  }

  // read response
  uint8_t response = 0x0;
  assert(sizeof(response) == RESPONSE_SIZE);
  if (read(sockfd, &response, sizeof(response)) == -1) {
    error_exit("read");
  }
  uint8_t nok = response >> 7;           // get MSB
  uint8_t value = (response << 1) >> 1;  // remove MSB

  // print server response (don't modify code below)
  puts((nok) ? "NOK" : "OK");
  if (arguments.cmd == GET && !nok) {
    printf("%d\n", value);
  }

  close(sockfd);
  exit(EXIT_SUCCESS);
}
