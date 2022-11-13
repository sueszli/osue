# @header Makefile for mycompress
# @author Josef Rabmer 11911128
# @version 0.1
# @date 2021-11-14
#
# @copyright Copyright (c) 2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mycompress.o compression.o

.PHONY: all clean
all: mycompress

mycompress: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mycompress.o: mycompress.c compression.h
compression.o: compression.c compression.h

clean:
	rm -rf *.o mycompress