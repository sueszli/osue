#!/bin/sh

ARGS="GB29NWBK60161331926819"
ARGS="AT00123456789012345678"
ARGS="AT1800"
ARGS="DE3600"
ARGS="GB22YK"
ARGS=""

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./validate $ARGS

make clean
