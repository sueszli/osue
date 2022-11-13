/**
 * @file memMngmt.c
 * @author: Niki Herl, MatrNr. 01634238
 * @date: 23.10.2019
 * @brief memory management utility that allows to register pointers directly after allocing and closing them all in the end
 * @details
 *  maintains lists (actually arrays) of pointers to be free'd (or, in a seperate list: fclose'd)
 *  allows to simply register ("put") as well as change pointers (e.g. after a realloc, to avoid free-ing the old pointer)
 */

#include <stdio.h> // Windows version: #include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h> // for NULL
#include <unistd.h>
#include <assert.h>
#include <netdb.h>
// #include <glib.h>

// local declarations
#include "memMngmt.h"
#include "main.h"


const char *progName_P;
const char *progOptions_P;

void abortWMsgNStatCode(const char *msg_P, int statusCode, bool printErrno) {
    fprintf(stderr, "%s: %s\n", progName_P, msg_P);
    if (printErrno) {
        perror(NULL);
    }

    cleanup2();

    exit(statusCode);
}

void abortWMsg(const char *msg_P, bool printErrno) {
    abortWMsgNStatCode(msg_P, EXIT_FAILURE, printErrno);
}

void abortW2Msgs(const char *msg1_P, const char *msg2_P, bool printErrno) {
    fprintf(stderr, "%s:\n%s\n%s\n", progName_P, msg1_P, msg2_P);
    if (printErrno) {
        perror(NULL);
    }

    // this could possibly be better placed above printing the errors (client.c did it...)
    cleanup2();

    exit(EXIT_FAILURE);
}

// I got this pattern and the 2,3 lines using it in main from https://jameshfisher.com/2017/04/05/set_socket_nonblocking/
int minus_1_guard(int n, const char *const err) {
    if (n == -1) { abortWMsg(err, true); }
    return n;
}

void *null_guard(void *ptr, const char *const err) {
    if (ptr == NULL) { abortWMsg(err, true); }
    return ptr;
}

int not_0_guard(int n, const char *const err) {
    if (n != 0) { abortWMsg(err, true); }
    return n;
}

int negative_guard(int n, const char *const err) {
    if (n < 0) { abortWMsg(err, true); }
    return n;
}