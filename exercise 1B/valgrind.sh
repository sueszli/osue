#!/bin/bash

make supervisor
make generator

valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s \
./generator
