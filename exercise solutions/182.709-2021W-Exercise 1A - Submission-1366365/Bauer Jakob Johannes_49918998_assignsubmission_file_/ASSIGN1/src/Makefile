# Makefile
# Program Name: mygrep
# Author: BAUER Jakob Johannes, 12002215
# EMail: jakob.bauer@tuwien.ac.at

all: mygrep

mygrep: mygrep.o
	gcc -o mygrep mygrep.o

mygrep.o: mygrep.c
	gcc -Wall -g -std=c99 -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -c -o mygrep.o mygrep.c

clean:
	rm -rf *.o mygrep