##
#@file Makefile
#@author: Niki Herl, MatrNr. 01634238
#@date: 15.10.2019
#@brief Makefile for the mycompress program
##

# compiler command:
CC = gcc
# compiler flags:
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean

all: mycompress

mycompress: mycompress.o parse_usage.o memMngmt.o
	$(CC) -o $@ $^ -lm

mycompress.o: mycompress.c
	$(CC) -c $(CFLAGS) -o $@ $<

parse_usage.o: parse_usage.c
	$(CC) -c $(CFLAGS) -o $@  $<

memMngmt.o: memMngmt.c
	$(CC) -c $(CFLAGS) -o $@  $<

clean:
	rm *.o mycompress
