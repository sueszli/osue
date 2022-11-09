#!/bin/bash

make ispalindrom

valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s \
./ispalindrom \
-s -i \
--outfile "./test data/output.txt" \
"./test data/input1.txt" "./test data/input2.txt"
