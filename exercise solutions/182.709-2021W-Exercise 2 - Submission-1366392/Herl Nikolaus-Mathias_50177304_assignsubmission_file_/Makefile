##
#@file Makefile
#@author: Niki Herl, MatrNr. 01634238
#@date: 12021-12-06 Human Era
#@brief Makefile for the cpair program
##

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LINKMATH = -lm

.PHONY: all clean

all: cpair

cpair: main.o memMngmt.o PointList.o datatypes.o
	$(CC) -o $@ $^ $(LINKMATH)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm *.o cpair
