#@file: Makefile
#@author: Jan Leszczyk 12019835
#@date 31.10.2021

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=2008090L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean

all: mygrep

mygrep: mygrep.o
	$(CC) -o $@ $^

mygrep.o: mygrep.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mygrep