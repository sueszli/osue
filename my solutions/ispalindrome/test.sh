#!/bin/bash

echo -e "\n\n--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./ispalindrome -s -i -o output.txt input1.txt input2.txt

echo -e "\n\n--------- CLEANING ---------"
make clean
