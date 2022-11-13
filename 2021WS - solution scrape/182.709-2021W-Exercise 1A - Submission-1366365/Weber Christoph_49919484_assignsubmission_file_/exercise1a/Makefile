# @author Christoph Weber 11908087
# @file ispalindrom Makefile

CC = gcc

.PHONY: all clean

all: ispalindrom

flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

ispalindrom.o: ispalindrom.c
	$(CC) $(flags) -c ispalindrom.c

ispalindrom: ispalindrom.o
	$(CC) -o ispalindrom ispalindrom.o

clean:
	rm -f ispalindrom ispalindrom.o