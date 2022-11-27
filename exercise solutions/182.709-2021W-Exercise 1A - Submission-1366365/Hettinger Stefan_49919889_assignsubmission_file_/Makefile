#@file Makefile
#@author Stefan Hettinger <e11909446@student.tuwien.ac.at>
#@date 23.10.2021
#@brief Makefile for UE1A: ispalindrom

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

OBJECTS = ispalindrom.o

.PHONY: all clean
all: ispalindrom

ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ispalindrom.o: ispalindrom.c

clean:
	rm -rf *.o ispalindrom