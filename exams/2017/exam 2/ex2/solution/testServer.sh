#!/bin/sh

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server

make clean

# echo "Content of '/dev/shm':"
# cd "/dev/shm"
# ls
