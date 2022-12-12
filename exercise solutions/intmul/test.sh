#!/bin/bash

ARGS="00Deadbe\n000000ef"

# examples
#ARGS="3\n3"
#ARGS="1A\nB3"
#ARGS="1000\n0001"
#ARGS="13A5D87E85412E5F\n7812C53B014D5DF8"

# mandatory test cases: valid inputs
#ARGS="3\n4"
#ARGS="3\n3"
#ARGS="1A\nb3"
#ARGS="1000\n0001"
#ARGS="aB\n7"
#ARGS="Deadbe\nef"
#ARGS="13A5D87E85412E5F\n7812C53B014D5DF8"

# mandatory test cases: invalid inputs
#ARGS="1"
#ARGS="1\nx"
#ARGS="2\n2.0"

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
echo -e $ARGS | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./intmul

echo -e "\n\n--------- RETURN STATUS ---------"
echo -e "(success == 0, failure == 1)"
echo $?


echo -e "\n\n--------- CLEANING ---------"
make clean
