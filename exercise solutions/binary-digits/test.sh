#!/bin/bash

ARGS=""
ARGS="-d test"
ARGS="-d -o test"
ARGS="-d 10 test"
ARGS="-d 10.12345 test"
ARGS="-d 10 -o ./text.txt path"
ARGS="-d 10 ./text1.txt"
ARGS="-d 10 ./text1.txt ./text2.txt"

clear

echo -e "--------- COMPILING ---------"
gcc -o binary-digits binary-digits.c -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./binary-digits $ARGS

echo -e "\n\n--------- CLEANING ---------"
	rm -rf *.o binary-digits
