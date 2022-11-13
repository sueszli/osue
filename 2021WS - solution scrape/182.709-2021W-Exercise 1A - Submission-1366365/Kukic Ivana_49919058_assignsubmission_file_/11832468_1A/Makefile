#Author 11832468 - Ivana Kukic
#Program name: mycompress

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all : mycompress

mycompress.o : mycompress.c
	$(CC) -o mycompress.o mycompress.c -c $(CFLAGS)

mycompress : mycompress.o
	$(CC) -o mycompress mycompress.o $(CFLAGS)

clean :
	-rm -f mycompress.o
	-rm -f mycompress
