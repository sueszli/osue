#!/bin/sh

ELF="./solution0"
ARGS="-gp 41 43"
ARGS="-p 1 -g 2  "
ARGS="-gp 41 -s 43 44"
ARGS="-p 41 -s 43 44"

# --------------------------------------------

ELF="./solution1"
ARGS=""

# --------------------------------------------

clear

FLAGS="-std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L"
gcc-12 -o $ELF $FLAGS $ELF.c 

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s $ELF $ARGS

rm -rf $ELF
