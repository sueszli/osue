#!/bin/bash

# ARGS="1-4 1-5 2-6 3-4 3-6 4-5 6-0 6-5"
# ARGS="0-1 1-2 2-0"
# ARGS="0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0"

ARGS="0-2 0-9 0-b 1-4 3-2 3-6 4-2 4-9 5-2 5-b 6-2 6-4 7-2 7-4 7-5 7-8 7-g 7-h 8-9 8-c 8-h a-2 a-9 b-2 c-1 c-6 c-a d-5 d-6 d-8 e-4 e-c f-8 f-b f-d g-1 g-6 g-h h-6 h-a h-b i-7 i-8 i-b"

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./generator $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
