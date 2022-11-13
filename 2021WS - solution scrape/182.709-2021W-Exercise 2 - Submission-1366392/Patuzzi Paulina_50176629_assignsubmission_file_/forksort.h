/**
 * @file forksort.h
 * @author Paulina Patuzzi (01607360)
 * @date 2020-12-08
 *
 * @brief Implementation of forksort module
 * */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

/**
 * Program entry point.
 * @brief Program starts here.
 * @details sorts the lines of a given file by recursively and calling itself with (about) half the lines
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS on success and EXIT_FAILURE otherwise
 * */
int main(int argc, char *argv[]);