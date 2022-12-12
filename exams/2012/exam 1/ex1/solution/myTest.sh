#!/bin/bash

ARGS=""

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./listtool $ARGS

make clean
