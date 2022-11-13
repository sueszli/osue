CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

all: ispalindrom clean

ispalindrom: ispalindrom.o
	$(CC) $(CFLAGS) -o ispalindrom ispalindrom.o

ispalindrom.o: ispalindrom.c ispalindrom.h
	$(CC) $(CFLAGS) -c -o ispalindrom.o ispalindrom.c

clean: 
	rm -rf *.o 
