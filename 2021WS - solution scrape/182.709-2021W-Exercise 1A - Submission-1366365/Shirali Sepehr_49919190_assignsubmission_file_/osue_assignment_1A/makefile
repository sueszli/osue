
.PHONY: all clean

all: ispalindrom

ispalindrom: ispalindrom.o
	gcc -o ispalindrom ispalindrom.o

ispalindrom.o: ispalindrom.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c ispalindrom.c

clean:
	rm -rf ispalindrom ispalindrom.o
