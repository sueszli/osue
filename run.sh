#!/bin/bash

make ispalindrom
printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"

./ispalindrom -s -i --outfile ./output.txt ./input1.txt ./input2.txt

printf "−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−\n"
make clean

# valgrind --leak-check=full --show-leak-kinds=all -s ./ispalindrom -s -i --outfile ./output.txt ./input1.txt ./input2.txt
