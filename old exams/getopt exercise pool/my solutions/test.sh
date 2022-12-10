#!/bin/sh

# exercise 0:
ARGS=""
ELF="./exercise0"

# exercise 1:

# exercise 2:

# exercise 3:


echo -e "--------- COMPILING ---------"
FLAGS = "-std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L"
gcc -o $FLAGS 

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s $ELF $ARGS

echo -e "\n\n--------- CLEANING ---------"
rm -rf ...