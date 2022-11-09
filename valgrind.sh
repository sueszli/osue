#!/bin/bash

make ispalindrom

valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s \
./ispalindrom \
-s -i \
# --outfile ./output.txt ./input1.txt ./input2.txt
