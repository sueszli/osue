#!/bin/bash

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./ispalindrome -s -i -o \
"./test data/output.txt" "./test data/input1.txt" "./test data/input2.txt"
echo " "
cat "./test data/output.txt"

echo -e "\n\n--------- CLEANING ---------"
make clean
