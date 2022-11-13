CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

.PHONY: all clean

all: mygrep

mygrep: main.o
	$(CC) -o $@ main.o

main.o: main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm mygrep main.o