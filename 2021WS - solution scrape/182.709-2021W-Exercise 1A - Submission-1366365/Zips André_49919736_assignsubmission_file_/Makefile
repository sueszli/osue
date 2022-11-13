# Makefile for myexpand
# Author: Andre Zips 11811363
.Phony: all clean

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all: myexpand.o
	$(CC) -o myexpand myexpand.o

myexpand.o:
	$(CC) $(CFLAGS) myexpand.c

clean: 
	rm myexpand.o
	rm myexpand
