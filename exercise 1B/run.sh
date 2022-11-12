#!/bin/bash
# @file Bash script to run and test the application.
# @author Yahya Jabary <mailto: yahya.jabary@tuwien.ac.at>
# @date 01.11.2022
# 
# @brief Run the application with testing data with and without the most verbose valgrind output.

# graphs ::
ARGS="1-4 1-5 2-6 3-4 3-6 4-5 6-0 6-5"
ARGS="0-2 0-9 0-11 1-4 3-2 3-6 4-2 4-9 5-2 5-11 6-2 6-4 7-2 7-4 7-5 7-8 7-16 7-17 8-9 8-12 8-17 10-2 10-9 11-2 12-1 12-6 12-10 13-5 13-6 13-8 14-4 14-12 15-8 15-11 15-13 16-1 16-6 16-17 17-6 17-10 17-11 18-7 18-8 18-11"
ARGS="0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0"
ARGS="0-1 1-2 2-0"
ARGS="0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0"
# :: graphs

clear

make supervisor
make generator

printf "\n███████████████████████████████████████████████████████████████████████████████████████\n"
printf "\nRUNNING SUPERVISOR\n"

valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s \
./supervisor

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"
printf "\nRUNNING GENERATOR\n"

valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s \
./generator $ARGS

printf "\n███████████████████████████████████████████████████████████████████████████████████████\n"
printf "\nRUNNING ALL PROCESSES\n"

./supervisor

./generator $ARGS

printf "\n███████████████████████████████████████████████████████████████████████████████████████\n"

make clean
