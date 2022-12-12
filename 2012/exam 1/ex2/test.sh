#!/bin/bash
SERVER=./server
CLIENT=./client

function info() {
    echo -e "\033[36m\nINFO: $* \033[0m" >&2
}
function testinfo() {
    export TEST=$*
    echo -e "\033[36m\nRUNNING TEST: ${TEST} \033[0m" >&2
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
   if [ "$EXP" -eq 0 -a "$RET" -ne 0 ] ; then
       error "${EMSG}"
   elif [ "$EXP" -ne 0 -a "$RET" -eq 0 ] ; then
       error "${EMSG}"
   else
       ok ${TEST}
   fi
}

function start_server() {
    info "Starting Server $1, with sequence number $2"
    ${SERVER} -p 0 -n $1 -s $2 >$1.log &
    sleep 1
    port=$(head -n 1 $1.log | cut -d' ' -f 4)
    echo ${port}
}

PORT1=$(start_server server-1 100)
if [ -z "${PORT1}" ] ; then
    error "Failed to startup server"
    exit 1
else
    ok "Server server-1 listening on port ${PORT1}"
fi
PORT2=$(start_server server-2 200)
if [ -z "${PORT2}" ] ; then
    error "Failed to startup server"
    exit 1
else
    ok "Server server-2 listening on port ${PORT2}"
fi

testinfo "Argument handling: neither -r nor -s"
${CLIENT} -p $PORT1
ret_check $? 1 "missing option -r or -s, but program returned EXIT_SUCCESS"

testinfo "Argument handling: additional arguments"
${CLIENT} -p $PORT1 -r server-1
ret_check $? 1 "additional positional arguments, but program returned EXIT_SUCCESS"

testinfo "Issue request (main server-1)"
${CLIENT} -p $PORT1 -r | tee /dev/stdout | grep "server-1 100" >/dev/null
ret_check $? 0 "Expected output: server-1 100"

testinfo "Issue request (main server-1, backup server-2)"
${CLIENT} -p $PORT1 -b $PORT2 -r | tee /dev/stdout | grep "server-1 101" >/dev/null
ret_check $? 0 "Expected output: server-1 101"

testinfo "Issue request (main server-2, no backup)"
${CLIENT} -p $PORT2 -r | tee /dev/stdout | grep "server-2 200" >/dev/null
ret_check $? 0 "Expected output: server-2 200"

testinfo "Shutdown server-1"
${CLIENT} -p $PORT1 -s && ok "./client: EXIT_SUCCESS" || error "shutdown failed"

testinfo "Issue request (main server-1, backup server-2)"
${CLIENT} -p $PORT1 -b $PORT2 -r | tee /dev/stdout | grep "server-2 201" >/dev/null
ret_check $? 0 "Expected output: server-2 201"

testinfo "Shutdown server-2"
${CLIENT} -p $PORT2 -s && ok "./client: EXIT_SUCCESS" || error "shutdown failed"

testinfo "Issue request (main server-1, backup server-2) - should fail"
${CLIENT} -p $PORT1 -b $PORT2 -r
ret_check $? 1 "Request should fail (both servers down)"

info "Test finished"