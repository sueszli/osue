#!/bin/bash

ARGS="test"
ARGS="http://"
ARGS="http://tester.com?test=1"

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./client $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
