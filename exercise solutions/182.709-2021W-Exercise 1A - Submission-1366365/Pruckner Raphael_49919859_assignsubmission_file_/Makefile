# @file Makefile
# @author Raphael Pruckner
# @date 11.11.2021
#
# @brief Makefile for compiling mycompress

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean
all: mycompress

mycompress: mycompress.o
	$(CC) $(CLFAGS) -o $@ $<

mycompress.o: mycompress.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mycompress