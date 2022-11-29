#!/bin/bash

echo -e "\n\n--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./supervisor

echo -e "\n\n--------- CLEANING ---------"
make clean
