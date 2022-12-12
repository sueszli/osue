#!/bin/sh

ARGS=""
ARGS="-g 1337"
ARGS="-g 13"
ARGS="-g -s 13"
ARGS="-s 13"
ARGS="-s 13 14"
ARGS="-s 128 14"
ARGS="-s 127 14"
ARGS="-s 127 -p 14"
ARGS="-s 127 -p 1024 62"

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client $ARGS

make clean
