# -*- MakeFile -*-

CC = gcc
CFLAGS =  -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g 

.PHONY: all clean

all: mygrep clean

mygrep: mygrep.o
	$(CC) -o $@ $^

mygrep.o: mygrep.c mygrep.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf mygrep.o
