#!/bin/bash

<<TEST_CASES
User input validation in test-cases 1-5 and 22-23:
  - no arguments
  - port: abc
  - port: 80z
  - using -p twice
  - non existent option -a
  - port: -2
  - port: 65599

-----

Server for test-cases 6-9:
  mkdir -p docroot; rm docroot/*
  echo "Hello world." > docroot/hello.html
  echo "Index." > docroot/index.html
  echo "Main." > docroot/main.file
  echo "12345" > docroot/123
  ./testServer.sh

Preparing server for test-cases 10-17:
  mkdir -p docroot; rm docroot/*
  ./testServer.sh

Preparing server for test-cases 18-19:
  mkdir -p docroot; rm docroot/*
  echo "Hello world." > docroot/hello.html
  echo "12345" > docroot/123
  ./testServer.sh

Preparing server for test-cases 20:
  mkdir -p docroot; rm docroot/*
  ( for i in `seq 1 10`; do echo "ABC$i" | sha1sum | cut -d " " -f1 | tr -d \\n; printf -- "-%.0s" {1..8000}; echo "__end$i."; done; ) > docroot/longlines.html
  sha1sum docroot/longlines.html
  ./testServer.sh

Preparing server for test-cases 21:
  mkdir -p docroot; rm docroot/*
  echo -n "01234567890abcdefghijklmnopqr" > chars
  echo -n "stuvwxyz01234567890abcdefghij" >> chars
  echo -n "klmnopqrstuvwxyz.,:-!=?% ABCD" >> chars
  echo -n "EFGHIJKLMNOPQRSTUVWXYZABCDEFG" >> chars
  echo -n "HIJKLMNOPQRSTUVWXYZ" >> chars
  ( for i in `cat chars | sed "s/./\0\n/g"`; do echo $i | sha1sum | tr -d "\n"; echo -n "x"; cat chars | sed "s/./\0$i/g"; echo ""; done; ) > docroot/mixed.html
  sha1sum docroot/mixed.html
  ./testServer.sh

Preparing sever for test-case 52 (content type):
  mkdir -p docroot; rm docroot/*
  echo "content of index.html." > docroot/index.html
  echo "content of test.htm." > docroot/test.htm
  echo "content of style.css." > docroot/style.css
  echo "content of lib.js." > docroot/lib.js
  echo "content of other.file." > docroot/other.file
  ./testServer.sh

Preparing sever for optional test case:
  mkdir -p docroot; rm docroot/*
  echo "Index." > docroot/index.html
  ./testServer.sh

TEST_CASES


ARGS="-p 9999 -i main.file docroot"
ARGS="-p 9991 docroot"



clear

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./server $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
