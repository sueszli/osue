#ifndef COMMON_H_
#define COMMON_H_

/** textual representation of the server's IP address */
#define SERVER_IPADDR_STR   "127.0.0.1"

/** default port number */
#define DEFAULT_PORTNUM     (2017)
#define DEFAULT_PORTNUM_STR "2017"

/** size of the request packet in bytes
 * device id + command (1), value (1) */
#define REQUEST_SIZE	(2)

/** size of the response packet in bytes
 * nok + value (1) */
#define RESPONSE_SIZE	(1)

/** Defines the command. */
typedef enum {
    GET = 0,
    SET = 1,
    UNDEF
} cmd_t;

/** Prints an error message and exits with EXIT_FAILURE. If errno is not 0,
 * also strerror(errno) is printed. */
void error_exit(const char *msg);

#endif /* COMMON_H_ */
