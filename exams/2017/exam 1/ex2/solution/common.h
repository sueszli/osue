#ifndef COMMON_H_
#define COMMON_H_

// servers ip address
#define SERVER_IPADDR_STR   "127.0.0.1"

// default port number
#define DEFAULT_PORTNUM     (2017)
#define DEFAULT_PORTNUM_STR "2017"

// request packet size in bytes: device id + command (1), value (1)
#define REQUEST_SIZE	(2)

// response packet size in bytes: nok + value (1)
#define RESPONSE_SIZE	(1)

// command
typedef enum {
    GET = 0,
    SET = 1,
    UNDEF
} cmd_t;

void error_exit(const char *msg);

#endif
