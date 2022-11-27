//
// Created by NikiH on 14.4.19.
//

#define Reg_free 0 // for malloc, ...
#define Reg_close 10 // for file descriptors
#define Reg_fclose 100 // for streams, i.e. after stdio.h-function use like fopen
#define Reg_freeaddrinfo 200

#include <stdbool.h>

/**
 * does internal data-structure setup and sets strings for usage-messages
 * @param progName
 * @param progOptions
 */
void initializeMemMngmt(const char *progName, const char *progOptions);

/**
 * TODO: seperate "register" and "register if not already present".
 * adds P to regType-list if it's not in the list yet
 * post: P *will* get 'freed' upon calling cleanup or abortXXX
 * @param P
 * @param regType
 * @return nothing - aborts with a message in failure case
 */
void registerPtr(void *const P, const int regType);

/**
 * replaces old_P in regType-list with new_P
 * @param old_P
 * @param new_P
 * @param regType
 * @return nothing - aborts with a message in failure case
 */
void registerPtrChange(const void *const old_P, void *const new_P, const int regType);

/**
 * frees/closes all the registered pointers/streams/etc.
 */
void cleanup(void);

/**
 * write usage error to stderr, perform cleanup, exit programm
 */
void abortWUsageError(void);

/**
 * @param msg_P
 * @param statusCode
 * @param printErrno
 */
void abortWMsgNStatCode(const char *msg_P, int statusCode, bool printErrno);

/**
 * same as abortWMsgNStatCode, but with fixed statusCode EXIT_FAILURE (= 1)
 * @param msg_P
 * @param printErrno
 */

void abortWMsg(const char *msg_P, bool printErrno);


/**
 * @param msg1_P
 * @param msg2_P
 * @param printErrno
 */
void abortW2Msgs(const char *msg1_P, const char *msg2_P, bool printErrno);

/**
 * @param P
 * @param P_name
 * @param Reg_type
 */
void check_null_and_register_or_fail(void *const P, const char *const P_name, const int Reg_type);

/**
 * @param old_P
 * @param new_P
 * @param regType
 */
void registerOrChange(void *old_P, void *new_P, int regType);

/**
 * @param n
 * @param err msg
 * @return n if n isn't minus 1. aborts with err msg if it is
 */
int minus_1_guard(int n, const char *const err);

/**
 * guards against ptr being null, i.e. aborts with err msg if it is
 * @param ptr
 * @param err msg
 * @return ptr if ptr isn't NULL. aborts with err msg if it is
 */
void *null_guard(void *ptr, const char *const err);

/**
 * @param n
 * @param err msg
 * @return n if n is 0. aborts with err msg if it isn't
 */
int not_0_guard(int n, const char *const err);

/**
 * @param n
 * @param err msg
 * @return n if n is 0. aborts with err msg if it isn't
 */
int negative_guard(int n, const char *const err);
