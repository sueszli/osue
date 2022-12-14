#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdint.h>

typedef enum { GET = 0, SET = 1, UNDEF = 2 } cmd_t;

struct args {
  uint16_t portnum;     // < port number [1024;UINT16_MAX]
  const char *portstr;  // < port number as string
  cmd_t cmd;            // < command (GET, SET)
  uint8_t value;        // < set value [0;127]
  uint8_t id;           // < device id [0;63]
};

void usage(void);

long parse_number(const char *str);

void apply_command(struct args arguments);

#endif
