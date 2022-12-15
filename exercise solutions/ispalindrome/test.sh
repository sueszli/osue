#!/bin/bash

clear

echo -e "--------- COMPILING ---------"
gcc -o ispalindrome ispalindrome.c -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./ispalindrome -s -i -o "./test data/output.txt" "./test data/input1.txt" "./test data/input2.txt"
echo " "
cat "./test data/output.txt"

echo -e "\n\n--------- CLEANING ---------"
	rm -rf *.o ispalindrome
