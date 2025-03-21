ARGS=""

# -------

clear

gcc -o fifo fifo.c -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./fifo $ARGS

rm -rf *.o fifo

echo "Content of '/dev/shm':"
cd "/dev/shm"
ls
