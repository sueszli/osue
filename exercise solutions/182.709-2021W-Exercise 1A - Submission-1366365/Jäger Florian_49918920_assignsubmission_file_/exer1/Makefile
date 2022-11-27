#Author: Florian Jäger 
#Program: ispalindrom
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

.PHONY: all clean
all: ispalindrom

ispalindrom: ispalindrom.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -lpthread -lrt

ispalindrom.o: ispalindrom.c ispalindrom.h


clean:
	rm -rf ispalindrom ispalindrom.o
