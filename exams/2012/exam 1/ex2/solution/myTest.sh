#!/bin/bash

PORT=9999
PORT=1337
PORT=8888
PORT=8080

# first call server in another terminal:
# Usage: ./server -p port [-n <name>] [-s <seqnr>]
# ie. clear && ./server -p 8080 -n swaggy -s 1337 

# then call the client with this test:
# Usage: -p PORT [-b PORT] [-r|-s]
ARGS="-p ${PORT} -r"

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./client $ARGS

make clean
