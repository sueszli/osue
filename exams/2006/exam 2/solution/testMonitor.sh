# prog: should write something to stderr and stdout
# stderr content gets written to console
# stderr content gets sent to stdin of log

ARGS="echo echo"

# -------

clear

gcc -o monitor monitor.c -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./monitor $ARGS

rm -rf *.o monitor
