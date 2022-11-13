#@file Makefile of cpair
#Author: Klaus Trimbacher 11908086
#Date: 11.12.2021

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

OBJECTSSUB = intmul.o

all:intmul

intmul: $(OBJECTSSUB)
	$(CC) $(LDFLAGS) -o $@ $^ -lm
	
intmul.o: intmul.c
	$(CC) $(CFLAGS) -c -o $@ $< -lm
	
clean:
	rm -rf *.o intmul