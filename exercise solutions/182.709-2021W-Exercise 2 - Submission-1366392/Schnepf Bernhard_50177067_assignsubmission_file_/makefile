# @file Makefile
# @author Bernhard Schnepf, 12022508
# @date 10.12.2021
# @brief Makefile for cpair

CC = gcc -g
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDLIBS = -lm #math.h

all: cpair

capair: cpair.o
		$(CC) -o capair capair.o $(LDLIBS)

cpair.o: cpair.c
		$(CC) $(CFLAGS) -c -o cpair.o cpair.c

clean:
		rm -rf *.o cpair cpair.tgz

tar:
		tar -cvzf cpair.tgz cpair.c makefile