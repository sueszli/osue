#!/bin/sh

# first run server:


# then run client:
# ./client [-p PORT] {-g|-s VALUE} I

ARGS=""

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client $ARGS

make clean
