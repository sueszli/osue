# Author: Laurenz Bilek, 11904655
# Program name: mygrep
CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mygrep.o

.PHONY: all clean
all: mygrep

mygrep: mygrep.o mygrepUtil.o
	$(CC) -o mygrep mygrep.o mygrepUtil.o

mygrep.o: mygrep.c
	$(CC) -g -c -o mygrep.o mygrep.c

mygrepUtil.o: mygrepUtil.c mygrepUtil.h
	$(CC) -g -c -o mygrepUtil.o mygrepUtil.c
	
clean:
	rm -rf *.o mygrep
