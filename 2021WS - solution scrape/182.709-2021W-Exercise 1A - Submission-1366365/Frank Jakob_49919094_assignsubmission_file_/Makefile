# @file Makefile
# @author Jakob Frank (11837319)
# @date 07.11.2021

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
OBJECTS = mycompress.o

all: mycompress

mycompress: mycompress.o
	$(CC) -o $@ $^

mycompress.o: mycompress.c
	$(CC) $(CFLAGS) mycompress.c

clean:
	rm -rf *.o mycompress
