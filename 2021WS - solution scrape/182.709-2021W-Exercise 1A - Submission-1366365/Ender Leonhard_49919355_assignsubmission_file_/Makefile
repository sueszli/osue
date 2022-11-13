# Makefile for exercise 1a
# author Leonhard Ender (12027408)
# date 13.11.2021

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = mycompress.o

all: mycompress

mycompress: mycompress.o
	$(CC) -o $@ $^

%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mycompress
