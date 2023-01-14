#!/bin/bash

# use: wget http://localhost:9999/index.html -O - | cat -e

ARGS=""
ARGS="-p 1337 -i testTwo testThree"
ARGS="-p 9999 ./serve"
ARGS=""

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./server $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
