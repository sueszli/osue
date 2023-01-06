#!/bin/bash

ARGS=""

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./server $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
