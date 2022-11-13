# Julia Bernold
# 22.11.2021
#
# The Makefile for intmul

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS =

.PHONY: all clean

all: intmul

intmul: main.o
	$(CC) $(LDFLAGS) -o intmul main.o
	
main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c
	
clean:
	rm -rf *.o intmul

