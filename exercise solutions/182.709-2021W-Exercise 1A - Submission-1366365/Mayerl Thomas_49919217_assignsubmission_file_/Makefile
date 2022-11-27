# Makefile for program mycompress
# Author: MAYERL Thomas, 52004289 <thomas.mayerl@tuwien.ac.at>

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mycompress.o

.PHONY = all clean
all: mycompress

mycompress: $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mycompress.o: mycompress.c

clean:
	rm -rf *.o mycompress