#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdint.h>

#include "common.h"

// arguments
struct args {
  uint16_t portnum;    // < port number
  const char *portstr; // < port number as string
  cmd_t cmd;           // < command (GET, SET)
  uint8_t value;       // < set value
  uint8_t id;          // < device id
};

void parse_arguments(int argc, char **argv, struct args *res);

void task_1_demo(int *sockfd, struct args *arguments);
void task_2_demo(int *sockfd, struct args *arguments, uint8_t *nok, uint8_t *value);

#endif
