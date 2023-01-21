#!/bin/sh

<<tests

compile and run server:
  clear && make all && ./server -p 8080 && make clean

run client based on `./client [-p PORT] {-g|-s VALUE} ID`
  ./client -s 50 24
  ./client -g 31

make clean

tests
