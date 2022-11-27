#author: Christoph Leopold
#program name: Makefile 
CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BDS_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
SOURCE=ispalindrom.c

all: ispalindrom.o
	$(CC) -o ispalindrom ispalindrom.o

ispalindrom.o: $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE)

clean:
	rm *.o ispalindrom
