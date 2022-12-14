#!/bin/bash

ARGS=""
ARGS="-s"
ARGS="-ssarg"
ARGS="-s sarg"
ARGS="-s test -a test"
ARGS="-a 2 test"
ARGS="-s -s"

<<initial_list
list[0]: head
list[1]: OSUE 2012
list[2]: having
list[3]: fun
list[4]: with
list[5]: lists
list[6]: and
list[7]: pointers
initial_list

ARGS="-s foo"

ARGS="-a 2 no"
ARGS="-a 4 funky"

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./listtool $ARGS

make clean
