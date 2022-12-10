#!/bin/sh

ELF="./solution0" # <-- change this to your executable
ARGS="" # <-- change this to your arguments

# --------------------------------------------

clear

FLAGS="-std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L"
gcc -o $ELF $FLAGS $ELF.c 

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s $ELF $ARGS

rm -rf $ELF
