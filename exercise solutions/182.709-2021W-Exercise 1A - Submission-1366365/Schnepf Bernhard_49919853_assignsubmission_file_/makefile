# @file Makefile
# @author Bernhard Schnepf, 12022508
# @date 03.11.2021
# @brief Makefile for mycompress

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

#.PHONY: all clean
all: mycompress

mycompress: mycompress.o
		$(CC) -o mycompress mycompress.o

mycompress.o: mycompress.c
		$(CC) $(CFLAGS) -c -o mycompress.o mycompress.c

clean:
		rm -rf *.o mycompress mycompress.tgz 

tar:
		tar -cvzf mycompress.tgz mycompress.c makefile