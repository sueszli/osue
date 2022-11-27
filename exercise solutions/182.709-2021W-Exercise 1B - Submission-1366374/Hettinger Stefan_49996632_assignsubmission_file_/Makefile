#@file Makefile
#@author Stefan Hettinger <e11909446@student.tuwien.ac.at>
#@date 26.10.2021
#@brief Makefile for UE1B: fb_arc_set

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o
	$(CC) $(LDFLAGS) -o $@ $^

generator: generator.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c supervisor.h

generator.o: generator.c supervisor.h

clean:
	rm -rf *.o supervisor generator