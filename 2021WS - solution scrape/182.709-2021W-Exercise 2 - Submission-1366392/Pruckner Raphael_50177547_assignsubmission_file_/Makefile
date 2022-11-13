# @file Makefile
# @author Raphael Pruckner
# @date 07.12.21
# @brief Makefile for compiling cpair

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS = -lm

.PHONY: all clean

all: cpair

cpair: cpair.o 
	$(CC) $(LDFLAGS) -o $@ $<

cpair.o : cpair.c cpair.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -rf *.o cpair