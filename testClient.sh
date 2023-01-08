#!/bin/bash

ARGS="test"
ARGS="http://"
ARGS="http://hostname.com/filename?test=1#test"
ARGS="http://hostname.com?test=1#test"
ARGS="http://hostname.com/"

ARGS="http://hostname.com/stuff/filename?test=1#test"
ARGS="http://hostname.com/stuff/filename?test=1#test"
ARGS="-o customFileName http://hostname.com/stuff/filename?test=1#test"
ARGS="-d test http://hostname.com/stuff/filename?test=1#test"

ARGS=""
ARGS="-p abc"
ARGS="-p 80x http://localhost/"
ARGS="-p 80 -p 81 http://localhost/"
ARGS="-a http://localhost/"
ARGS="-p 9999 http://localhost/hello.html"
ARGS="-p 9999 -d test1 http://localhost/"
ARGS="-p 9999 -o f.2 http://localhost/t"
ARGS="-p 9999 -o f.3 http://localhost/t"
ARGS="-p 9999 -d test.dir http://localhost/t"
ARGS="-p 9999 -d test2 http://localhost/m.php?a=3"
ARGS="-p 9999 -d test3 http://localhost?welcome=here"
ARGS="-p 9999 foo://localhost"
ARGS="-p 9999 http://"
ARGS="-p 9999 http://?getparam"
ARGS="-p 9999 http://localhost/hello.html"
ARGS="-p 9999 -o lines1.html http://127.0.0.1/longlines.html"
ARGS="-p 9999 -o lines2.html http://127.0.0.1/mixed.html"
ARGS="-p 9999 http://localhost/"
ARGS="-p -2 http://localhost/"
ARGS="-p 65599 http://localhost/"

ARGS="http://www.neverssl.com/"

echo -e "--------- COMPILING ---------"
make all

echo -e "\n\n--------- RUNNING WITH VALGRIND ---------"
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./client $ARGS

echo -e "\n\n--------- CLEANING ---------"
make clean
