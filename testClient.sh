#!/bin/bash

ARGS="test"
ARGS="http://"
ARGS="http://hostname.com"
ARGS="http://hostname.com/filename?test=1#test"
ARGS="http://hostname.com?test=1#test"
ARGS="http://hostname.com/"

ARGS="http://hostname.com/stuff/filename?test=1#test"
ARGS="-o customFileName http://hostname.com/stuff/filename?test=1#test"
ARGS="-d test http://hostname.com/stuff/filename?test=1#test"

ARGS="http://hostname.com/?test=1#test" # writes to stdout
ARGS="-d test http://hostname.com/?test=1#test"

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./client $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
