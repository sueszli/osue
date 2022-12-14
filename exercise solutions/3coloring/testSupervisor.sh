#!/bin/bash

clear

echo "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./supervisor

echo -e "\n\n--------- CLEANING ---------"
make clean
echo "Content of '/dev/shm':"
cd "/dev/shm"
ls
# sudo rm -rf "/dev/shm/" # should be done by supervisor if it doesn't crash
