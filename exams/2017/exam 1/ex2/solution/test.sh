#!/bin/sh

<<tests

compile and run server:
  make all && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server

run client based on `./client [-p PORT] {-g|-s VALUE} ID`
  ./client -s 50 24
  ./client -g 31


# -------

clear

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client $ARGS

make clean

tests
