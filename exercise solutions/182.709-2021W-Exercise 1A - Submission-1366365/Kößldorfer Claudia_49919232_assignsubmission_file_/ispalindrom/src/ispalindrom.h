/**
 * @file ispalindrom.h
 * @author Claudia Kößldorfer <e11825357@student.tuwien.ac.ar>
 * @date 30.10.2021
 *
 * @brief Reads and checks input files line by line for palindroms.
 * Returns each line and either "is a palindrom" or "is not a palindrom".
 * @details If -o is provided output will be written in the given output file otherwise it will be written to stdout.
 * An arbitrary number of input files can be given, which will be read from. If none are provided the program reads from stdin.
 * If -s is provided whitespaces will be ignored for the check.
 * If -i is provided the check will not differentiate between upper and lower case letters.
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

int main(int argc, char *argv[]);

