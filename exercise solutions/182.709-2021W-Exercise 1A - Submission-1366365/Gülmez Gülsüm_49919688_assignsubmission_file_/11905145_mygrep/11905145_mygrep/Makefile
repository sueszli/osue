# author: Gülsüm Gülmez (11905145)
# date: 08.11.2021
# program: mygrep

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean
all: mygrep

mygrep: mygrep.o
	$(CC) -o $@ $^

mygrep.o: mygrep.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mygrep

