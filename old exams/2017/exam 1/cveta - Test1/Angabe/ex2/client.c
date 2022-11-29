
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

#include "client.h"
#include "common.h"

/** name of the executable (for printing messages) */
char *program_name = "client";

/** program entry point */
int main(int argc, char **argv)
{
    struct args arguments;

    /* parse program arguments and fill 'arguments' [given] */
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
    int sockfd;

    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    /* file descriptor of socket */
    s = getaddrinfo(SERVER_IPADDR_STR, arguments.portstr, &hints, &ai);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sockfd == -1){
            error_exit("socket failed");
        }

        if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0){
            error_exit("connect failed");
        }

        /* REPLACE FOLLOWING LINE WITH YOUR SOLUTION */
       // task_1_demo(&sockfd, &arguments);

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

        uint8_t nok;
        uint8_t value;

        /* REPLACE FOLLOWING LINE WITH YOUR SOLUTION */
        //task_2_demo(&sockfd, &arguments, &nok, &value);

       
        char buffer[2];
        buffer[0] = (arguments.id << 2) | arguments.cmd;
        buffer[1] = arguments.value & 127;

        res = send(sockfd, buffer, 2, 0);
        if (res < 0)
            error_exit("");

        char buff[1];
        res = recv(sockfd, buff, 1, 0);
        if (res < 0)
            error_exit("");

        nok = buf[0] & 128;
        value = buf[0] & 127;

        /* DO NOT CHANGE THE FOLLOWING LINES */
        /* print server response */
        puts((nok) ? "NOK" : "OK");

        if (arguments.cmd == GET && !nok)
            printf("%d\n", value);

        /* cleanup: close socket */
        close(sockfd);

        exit(EXIT_SUCCESS);
    }
