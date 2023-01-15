#!/bin/bash

# use: wget http://localhost:9999/index.html -O - | cat -e
# use: wget http://localhost:9999/index.html -O - 2>/dev/null

ARGS=""
ARGS="-p 1337 -i testTwo testThree"
ARGS="-p 9999 ./serve"

ARGS=""
ARGS="-p abc ."
ARGS="-p 80z ."
ARGS="-p 80 -p 81"
ARGS="-a"
ARGS="-p 9999 docroot"
ARGS="-p 9999 -i main.file docroot"
ARGS="-p 9080 docroot"

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./server $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
