#!/bin/bash

ARGS=""
ARGS="-p 1337 -i testTwo testThree"
ARGS="./serve"
ARGS="-i na\:me ./ser:er"

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./server $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
