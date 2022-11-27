# Author: Valentin Futterer, 11904654
# Program name: mygrep

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean
all: mygrep

mygrep: mygrep.o mygrepUtil.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

client.o: mygrep.c mygrepUtil.h
util.o: mygrep.c mygrepUtil.h

clean:
	rm -rf *.o mygrep