#!/bin/sh

ARGS=""
ARGS="0- 0-2"
ARGS="0-1 0-1 0-2"
ARGS="0-1- 0-2"
ARGS="-11- 0-2"
ARGS="0-a 0-2"
ARGS="0-1 b-2"

ARGS="0-1 0-2 0-3 1-2 1-3 2-3"


clear

echo "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./generator $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
