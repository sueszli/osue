#!/bin/bash

ARGS="A\nAB3"

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
#ARGS="fe6975a937016c20b43b17540e6c6246cc5e9216b37b3f82dfe398ed311d16e63a0666c07bad1a857d8359a70dedb685\ndbd432ede8ea89ff7a24d6d7b47513c5c9dc7db2c2a84ccf1176c37e292185f7b79dafd9f1b32a27598872bba8a8f5da"

# mandatory test cases: invalid inputs
#ARGS="1"
#ARGS="1\nx"
#ARGS="2\n2.0"

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
echo -e $ARGS | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./intmul

echo -e "\n\n--------- CLEANING ---------"
make clean
