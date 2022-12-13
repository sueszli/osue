#!/bin/sh

ARGS=""

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./??? $ARGS

make clean
