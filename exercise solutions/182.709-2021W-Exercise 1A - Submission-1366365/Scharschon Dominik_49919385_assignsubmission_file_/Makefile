#@file Makefile
#
#@author Scharschon Dominik 01225876
#
#@brief Provides the makefile for ispalindrom.c
#
#@details Implement a program ispalindrom, which checks whether strings are palindroms
#
#@date 14.11.2021


CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS)
op = -g -c



all: ispalindrom

ispalindrom: ispalindrom.o
	$(CC) $(LDFLAGS) -lm

ispalindrom.o: ispalindrom.c
	$(CC) $(CFLAGS) -g -c ispalindrom.c

clean:
	rm -f ispalindrom *.o
