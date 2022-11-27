##Makefile for feedback-arc-set
##@author Gregor Käfer
##date 14-11-2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -lpthread

.PHONY: all clean

all:  supervisor generator


generator: generator.o generator.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

supervisor : supervisor.o supervisor.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
clean: 
	rm -rf *.o generator supervisor