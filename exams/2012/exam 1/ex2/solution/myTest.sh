#!/bin/bash

ARGS=""

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client $ARGS

make clean
