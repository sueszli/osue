#!/bin/sh

<<tests

compile and run server:
  make all && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server

run client based on `./client [-p PORT] {-g|-s VALUE} ID`
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client -s 50 24
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client -g 31

make clean

tests
