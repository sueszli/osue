#ifndef COMMON_H_
#define COMMON_H_

#define SERVER_IPADDR_STR   "127.0.0.1"
#define DEFAULT_PORTNUM     (2017)
#define DEFAULT_PORTNUM_STR "2017"

// request packet size in bytes: device id + command, value
#define REQUEST_SIZE	(2)

// response packet size in bytes: nok + value
#define RESPONSE_SIZE	(1)

typedef enum {
    GET = 0,
    SET = 1,
    UNDEF
} cmd_t;

void error_exit(const char *msg);

#endif
