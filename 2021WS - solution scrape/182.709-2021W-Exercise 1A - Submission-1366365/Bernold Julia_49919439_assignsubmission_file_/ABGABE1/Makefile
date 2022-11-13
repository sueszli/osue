#Julia Bernold
# 1.11.2021
#
#The Makefile for ispalindrom.c

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS =																		

.PHONY: all clean

all: ispalindrom

ispalindrom: main.o
	$(CC) $(LDFLAGS) -o ispalindrom main.o

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

clean:
	rm -rf *.o *.in *.out ispalindrom
