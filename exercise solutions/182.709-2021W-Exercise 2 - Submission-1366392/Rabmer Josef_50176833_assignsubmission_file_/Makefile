# @header Makefile for cpair project
# @author Josef Rabmer 11911128
# @version 0.1
# @date 2021-12-11
#
# @copyright Copyright (c) 2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm

OBJECTS = cpair.o

.PHONY: all clean
all: cpair

cpair: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 

cpair.o : cpair.c cpair.h

clean:
	rm -rf *.o cpair