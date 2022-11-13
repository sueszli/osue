CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mygrep.o

all: mygrep clean

mygrep: mygrep.o
	gcc -std=c99 -o mygrep mygrep.o

mygrep.o: mygrep.c mygrep.h
	gcc -std=c99 -c -o mygrep.o mygrep.c

clean:
	rm -rf *.o
