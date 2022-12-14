#!/bin/sh

ARGS="AT121234512345678901 -w 100"

# -------------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client $ARGS

make clean
