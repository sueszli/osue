#!/bin/bash

clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./supervisor

echo -e "\n\n--------- CLEANING ---------"
make clean
echo "Content of '/dev/shm':"
cd "/dev/shm"
ls
sudo rm -rf "/dev/shm/" # should happen automatically
