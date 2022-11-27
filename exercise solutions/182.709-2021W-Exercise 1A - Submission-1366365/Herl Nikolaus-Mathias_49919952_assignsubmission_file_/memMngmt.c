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

// local declarations
#include "memMngmt.h"

// #include <glib.h>

const long INITIAL_SIZE = 3;


typedef struct list {
    void **startPos;
    long curAllocN;
    long curElemN;

} listT;

/* TODO: properly implement
typedef struct int_list {
    void **startPos;
    long curAllocN;
    long curElemN;

} int_listT;
*/

listT freeList;
// int_listT closeList; TODO: properly implement
listT fcloseList;
listT freeaddrinfo_list;

listT *getListP(int regType) {
    switch (regType) {
        case Reg_free:
            return &freeList;
            /*
        case Reg_close:
            return &closeList;
            */
        case Reg_fclose:
            return &fcloseList;
        case Reg_freeaddrinfo:
            return &freeaddrinfo_list;
        default:
            abortWMsg("invalid regType was passed to memMngmt function", false);
    }
    assert(false);
}

const char *progName_P;
const char *progOptions_P;

// must be called before any other memMngmt functions are called.
void initializeMemMngmt(const char *progName, const char *progOptions) {
    progName_P = progName;
    progOptions_P = progOptions;

    freeList.startPos = calloc(INITIAL_SIZE, sizeof(void *));
    freeList.curAllocN = INITIAL_SIZE;
    freeList.curElemN = 0;

    /* TODO: properly implement
    closeList.startPos = calloc(INITIAL_SIZE, sizeof(void *));
    closeList.curAllocN = INITIAL_SIZE;
    closeList.curElemN = 0;
     */

    fcloseList.startPos = calloc(INITIAL_SIZE, sizeof(void *));
    fcloseList.curAllocN = INITIAL_SIZE;
    fcloseList.curElemN = 0;

    freeaddrinfo_list.startPos = calloc(INITIAL_SIZE, sizeof(void *));
    freeaddrinfo_list.curAllocN = INITIAL_SIZE;
    freeaddrinfo_list.curElemN = 0;
}

void registerPtr(void *const P, const int regType) {
    listT *list = getListP(regType);

    for (long i = 0; i < list->curElemN; i++) {
        if (list->startPos[i] == P) {
            abortWMsg("MemMngmt Problem: it was attempted to register a pointer twice", false);
        }
    }
    if (list->curAllocN == list->curElemN) {
        void **temp = realloc(list->startPos, 2 * list->curAllocN * sizeof(void *));
        null_guard(temp, "MemMngmt Problem: realloc for resizing ptr-list failed");

        list->curAllocN *= 2;
        list->startPos = temp;
    }

    list->startPos[list->curElemN] = P;
    list->curElemN++;
}

void registerPtrChange(const void *const old_P, void *const new_P, const int regType) {
    listT *list = getListP(regType);

    for (long i = 0; i < list->curElemN; i++) {
        if (list->startPos[i] == old_P) {
            list->startPos[i] = new_P;
            return;
        }
    }
    abortWMsg("registerPtrChange got to unexpected path", false);
}

void cleanup(void) {
    for (long i = 0; i < freeList.curElemN; ++i) {
        free(freeList.startPos[i]);
    }
    free(freeList.startPos);

    /* TODO: properly implement
    for (long i = 0; i < closeList.curElemN; ++i) {
        close((int) closeList.startPos[i]);
        // WARNING: I guess the (necessary) cast means super big file descriptors are not supported..
    }
    free(closeList.startPos);
    */

    for (long i = 0; i < fcloseList.curElemN; ++i) {
        not_0_guard(fclose(fcloseList.startPos[i]), "fclosing something failed");
    }
    free(fcloseList.startPos);

    for (long i = 0; i < freeaddrinfo_list.curElemN; ++i) {
        freeaddrinfo(freeaddrinfo_list.startPos[i]);
    }
    free(freeaddrinfo_list.startPos);
}

void abortWUsageError(void) {
    fprintf(stderr, "Usage: %s %s\n", progName_P, progOptions_P);
    cleanup();
    exit(EXIT_FAILURE);
}


void abortWMsgNStatCode(const char *msg_P, int statusCode, bool printErrno) {
    fprintf(stderr, "%s: %s\n", progName_P, msg_P);
    if (printErrno) {
        perror(NULL);
    }

    cleanup();

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
    cleanup();

    exit(EXIT_FAILURE);
}

void check_null_and_register_or_fail(void *const P, const char *const P_name, const int Reg_type) {
    if (P == NULL) {
        abortW2Msgs("initial allocation failed:", P_name, true);
    } else {
        registerPtr(P, Reg_type);
    }
}

void registerOrChange(void *old_P, void *new_P, int regType) {
    if (old_P == NULL) {
        registerPtr(new_P, regType);
    } else if (old_P != new_P) {
        registerPtrChange(old_P, new_P, regType);
    }
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