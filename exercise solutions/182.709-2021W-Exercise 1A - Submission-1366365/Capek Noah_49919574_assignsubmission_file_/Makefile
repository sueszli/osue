CC = gcc
CFLAGS=-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all:ispalindrom

ispalindrom: ispalindrom.o
	$(CC) -o ispalindrom ispalindrom.o

clean:
	rm -f *.o
	rm -f ispalindrom