#!/bin/bash

ARGS=""
ARGS="-s"
ARGS="-ssarg"
ARGS="-s sarg"
ARGS="-s test -a test"
ARGS="-a 2 test"

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./listtool $ARGS

make clean
