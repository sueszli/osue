#include <limits.h>
#include <signal.h>
#include "circularbuffer.h"

/**
 * @param sig this is the signal number wo which a handling function is set.
 * @brief initiates the termination sequence (signal handler)
 * @details set the termination boolean/flag to true, initializing the termination sequence
 * @return void
 * */
void sighandler(int sig);

/**
 * @param message error message
 * @brief terminates this programm
 * @details terminates this programm and prints this given error message
 * @return void
 * */
void terminate_with_error_2(char* message);