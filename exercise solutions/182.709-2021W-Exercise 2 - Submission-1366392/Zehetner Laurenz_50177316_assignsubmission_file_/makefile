# @brief Makefile for program: 'forksort'
# @author Laurenz Zehetner 12023972
# @date 11.12.2021


CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
CFLAGS = -std=c99 -pedantic -Wall $(DEFS)
LDFLAGS = 

.PHONY: all clean

all: forksort

forksort: forksort.o
	$(CC) -o $@ $^ $(LDFLAGS)

forksort.o:	forksort.c
	$(CC) $(CFLAGS) -c -o $@ $^



clean:
	rm -rf *.o forksort