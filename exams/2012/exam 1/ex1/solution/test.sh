#!/bin/bash

function info() {
    echo -e "\033[36m\nRUNNING TEST: $* \033[0m" >&2
}
function ok() {
    echo -e "\033[32mOK: $* \033[0m" >&2
}
function error() {
    echo -e "\033[33mERROR: $* \033[0m" >&2
}

function ret_check() {
   RET=$1
   EXP=$2
   shift
   shift
   EMSG=$*

   if [ $RET -eq $EXP ]; then
      ok ${MSG}
   else
      error ${EMSG}
      error "Fix your existing problems before we continue"
      exit
   fi
}

clear

make all

MSG="Argument handling: neither -s nor -a" && info ${MSG}
./listtool 2>/dev/null 
ret_check $? 1 "missing option -s or -a but program does not return  EXIT_FAILURE"

MSG="Argument handling: not allowed option (./listtool -x)" && info ${MSG}
./listtool -x 2>/dev/null
ret_check $? 1 "not allowed option (./listtool -x) but ./listtool does not return EXIT_FAILURE" 

MSG="Argument handling: valid call (./listtool -s foo)" && info ${MSG}
./listtool -s foo >/dev/null 2>&1
ret_check $? 0 "(./listtool -s foo) should be valid"

MSG="Argument handling: no option specified (./listtool foo)" && info ${MSG}
./listtool foo 2>/dev/null 
ret_check $? 1 "(./listtool foo) should be invalid but ./listtool does not return EXIT_FAILURE"

MSG="Argument handling: valid call (./listtool -a 0 foo)" && info ${MSG}
./listtool -a 0 foo >/dev/null 2>&1
ret_check $? 0 "(./listtool -a 0 foo) should be valid"

MSG="Argument handling: -s too often (./listtool -s foo -s bar)" && info ${MSG}
./listtool -s foo -s bar 2>/dev/null 
ret_check $? 1 "(./listtool -s foo -s bar) does not return EXIT_FAILURE"

MSG="Argument handling: -a too often (./listtool -a 0 foo -a 2 bar)" && info ${MSG}
./listtool -a 0 foo -a 2 bar 2>/dev/null 
ret_check $? 1 "(./listtool -a 0 foo -a 2 bar) does not return EXIT_FAILURE"

MSG="Argument handling: -a and -s (./listtool -a 0 foo -s bar)" && info ${MSG}
./listtool -a 0 foo -s bar 2>/dev/null 
ret_check $? 1 "(./listtool -a 0 foo -s bar) does not return EXIT_FAILURE"

MSG="I/O test exercise 1: ./listtool -s foo" && info ${MSG}
./listtool -s foo | egrep "^no,no,no,no,no,no,no,no,$" >/dev/null && ok ${MSG} || error "(./listtool -s foo) should print: no,no,no,no,no,no,no,no,"

MSG="I/O test exercise 1: ./listtool -s bar"
info ${MSG}
./listtool -s bar | egrep "^no,no,no,no,no,no,no,no,$" >/dev/null && ok ${MSG} || error "(./listtool -s bar) should print: no,no,no,no,no,no,no,no,"

MSG="I/O test exercise 1: ./listtool -s pointers"
info ${MSG}
./listtool -s pointers | egrep "^no,no,no,no,no,no,no,yes,$" >/dev/null && ok ${MSG} || error "(./listtool -s pointers) should print: no,no,no,no,no,no,no,yes,"

MSG="I/O test exercise 1: ./listtool -s fun"
info ${MSG}
./listtool -s fun | egrep "^no,no,no,yes,no,no,no,no,$" >/dev/null && ok ${MSG} || error "(./listtool -s fun) should print: no,no,no,yes,no,no,no,no,"

MSG="I/O test exercise 2: ./listtool -a 0 test"
info ${MSG}
./listtool -a 0 test | egrep "^chk: r5yW.Nwg/Ykdg$" >/dev/null && ok ${MSG} || error "(./listtool -a 0 test): List not correct (use print_list(head) for debugging.\nMake sure check_list(head) is called and that chk: <somehash> is your only output"

MSG="I/O test exercise 2: ./listtool -a 3 osue"
info ${MSG}
./listtool -a 3 osue | egrep "^chk: fii7/D/dnOqW2$" >/dev/null  && ok ${MSG} || error "(./listtool -a 3 osue): List not correct (use print_list(head) for debugging.\nMake sure check_list(head) is called and that chk: <somehash> is your only output" 

MSG="I/O test exercise 2: ./listtool -a 23 bar"
info ${MSG}
./listtool -a 23 bar | egrep "^chk: wmIW/Ll0Y5a9c$" >/dev/null && ok ${MSG} || error "(./listtool -a 23 bar): List not correct (use print_list(head) for debugging.\nMake sure check_list(head) is called and that chk: <somehash> is your only output"

echo -e "\nDONE: Test finished"

make clean
