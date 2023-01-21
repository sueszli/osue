
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

#include "client.h"
#include "common.h"

char *program_name = (char *)"client";

static int connectSocket(int connect_port, const char *address)
{
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

    if (!inet_aton(address, (struct in_addr *) &addr.sin_addr.s_addr)) {
        close(cfd);
        error_exit("inet_aton");
    }

    if (connect(cfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        shutdown(cfd, SHUT_RDWR);
        close(cfd);
        error_exit("connect");
    }

    return cfd;
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
    
    // print arguments
    printf("arguments: ");
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    // print parsed arguments
    printf("parsed arguments:\n");
    printf("\tportnum: %d\n", arguments.portnum);
    printf("\tportstr: %s\n", arguments.portstr);
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

    // create request bytes (in host order)
    union {
        struct {
            unsigned cmd : 2;
            unsigned id : 6;
        } fields;
        uint8_t all;
    } fst;
    fst.fields.cmd = arguments.cmd;
    fst.fields.id = arguments.id;

    union {
        struct {
            unsigned value : 7;
            unsigned : 1;
        } fields;
        uint8_t all;
    } snd;
    snd.fields.value = arguments.value;

    printf("request before merge:\n");
    printf("\tfst: ");
    for (int i = 7; i >= 0; i--) {
        if (i == 1) {
            printf(" ");
        }
        printf("%d", (fst.all >> i) & 1);
    }
    printf("\n");
    printf("\tsnd: ");
    for (int i = 7; i >= 0; i--) {
        if (i == 6) {
            printf(" ");
        }
        printf("%d", (snd.all >> i) & 1);
    }
    printf("\n");

    union {
        struct {
            uint8_t fst : 8;
            uint8_t snd : 8;
        } fields;
        uint16_t all;
    } request;
    request.fields.fst = fst.all;
    request.fields.snd = snd.all;

    printf("request after merge:\n\t");
    for (int i = 15; i >= 0; i--) {
        if (i == 7) {
            printf(" ");
        }
        printf("%d", (request.all >> i) & 1);
    }
    printf("\n");



    uint8_t value = 0x0;
    uint8_t nok = 0x0;

    // ----------------------

    // print server response (don't modify code below)
    puts((nok) ? "NOK" : "OK");
    if (arguments.cmd == GET && !nok) {
        printf("%d\n", value);
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}
