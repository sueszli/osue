#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdint.h>

/** Defines the command type. */
typedef enum {
  GET = 0,
  SET = 1,
  UNDEF = 2
} cmd_t;

/** Structure for the arguments. */
struct args {
  uint16_t portnum;   /**< port number */
  const char *portstr;/**< port number as string */
  cmd_t cmd;          /**< command (GET, SET) */
  uint8_t value;      /**< set value */
  uint8_t id;         /**< device id */
};

/** Prints the usage message and exits with EXIT_FAILURE. */
void usage(void);

/** Parses a string to a long integer. Returns -1 on error. */
long parse_number(const char *str);

/** Applies command according to arguments. */
void apply_command(struct args arguments);

#endif
