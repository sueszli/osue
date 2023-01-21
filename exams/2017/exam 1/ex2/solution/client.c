
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

int main(int argc, char **argv) {
    struct args arguments;
    parse_arguments(argc, argv, &arguments); // given


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

    /* file descriptor of socket */
    int sockfd;

    /* REPLACE FOLLOWING LINE WITH YOUR SOLUTION */
    task_1_demo(&sockfd, &arguments);


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
    task_2_demo(&sockfd, &arguments, &nok, &value);


    /* DO NOT CHANGE THE FOLLOWING LINES */
    /* print server response */
    puts((nok) ? "NOK" : "OK");

    if (arguments.cmd == GET && !nok)
        printf("%d\n", value);

    /* cleanup: close socket */
    close(sockfd);

    exit(EXIT_SUCCESS);
}
