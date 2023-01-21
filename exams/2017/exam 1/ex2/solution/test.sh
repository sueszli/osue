#!/bin/sh

<<tests

compile and run server:
  clear && make all && ./server -p 8080 && make clean

run client based on `./client [-p PORT] {-g|-s VALUE} ID`
  ./client -p 8080 -s 50 24
  ./client -p 8080 -g 31
  ./client -p 8080 -s 31 32

make clean

tests
