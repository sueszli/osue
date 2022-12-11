#!/bin/sh

ELF="./solution2"

<<solution0
ARGS="-gp 41 43"
ARGS="-p 1 -g 2  "
ARGS="-gp 41 -s 43 44"
ARGS="-p 41 -s 43 44"
ARGS="-p41 -s 43 44"
ARGS="-p"
solution0

<<solution1
ARGS=""
ARGS="-c"
ARGS="-c -s"
ARGS="-c -b test"
ARGS="-c -b test illegal"
ARGS="-c -btest -de"
ARGS="-ccarg -bbarg -de"
ARGS="-a arg -ccarg -bbarg -de"
ARGS="-a arg -ccarg -bbarg -derg"
ARGS="-aaarg -ccarg -bbarg -d"
ARGS="-ae -ccarg -bbarg -d"
solution1

ARGS=""
ARGS="-c"
ARGS="-c carg"

# ----------

clear

FLAGS="-std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L"
gcc-12 -o $ELF $FLAGS $ELF.c 

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s $ELF $ARGS

rm -rf $ELF
