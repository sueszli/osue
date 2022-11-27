#author: Jakob Kästner 01626126
#program: ispalindrom

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c 

all: build clean

build:
	$(CC) $(CFLAGS) ispalindrom.c
	$(CC) ispalindrom.o -o ispalindrom
clean:
	rm ispalindrom.o
