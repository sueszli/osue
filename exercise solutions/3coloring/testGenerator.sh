#!/bin/sh

ARGS=""
ARGS="0- 0-2"
ARGS="0-1 0-1 0-2"
ARGS="0-1- 0-2"
ARGS="-11- 0-2"
ARGS="0-a 0-2"
ARGS="0-1 b-2"

ARGS="0-1 0-2 0-3 1-2 1-3 2-3"
ARGS="0-1 0-3 0-4 1-2 1-3 1-4 1-5 2-4 2-5 3-4 4-5"
ARGS="0-1 0-2 0-3 1-28 1-29 2-30 2-31 3-32 3-33 4-6 4-14 4-16 5-7 5-15 5-17 6-7 6-18 7-19 8-9 8-12 8-23 9-13 9-22 10-15 10-19 10-25 11-14 11-18 11-24 12-17 12-27 13-16 13-26 14-23 15-22 16-21 17-20 18-21 19-20 20-31 21-30 22-33 23-32 24-27 24-29 25-26 25-29 26-28 27-28 30-33 31-32"


clear

echo "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./generator $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
