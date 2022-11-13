#!/bin/bash
# needs to be run with sudo so it can access /dev/shm

clear

make all

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"

valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s ./supervisor

# ./supervisor

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"

make clean

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"

echo "Content of '/dev/shm' - BEFORE:"
cd "/dev/shm"
ls -l
rm -rf "/dev/shm/"

echo "Content of '/dev/shm' - AFTER:"
cd "/dev/shm"
ls -l
cd "/mnt/c/Users/Yahya Jabary/dev/github/osue/exercise 1B"