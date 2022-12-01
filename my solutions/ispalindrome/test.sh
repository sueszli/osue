#!/bin/bash

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./ispalindrome -s -i -o output.txt input1.txt input2.txt
echo " "
cat ./output.txt

echo -e "\n\n--------- CLEANING ---------"
make clean
