# Author: Peter Kabelik (01125096)
# Program name: ispalindrom

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

all: ispalindrom

ispalindrom: ispalindrom.o
	$(CC) -o $@ $^
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf ispalindrom.o ispalindrom

.PHONY: all clean
