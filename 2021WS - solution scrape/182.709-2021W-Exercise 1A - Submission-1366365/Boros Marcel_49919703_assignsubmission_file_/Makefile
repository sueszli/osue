DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -c -std=c99 -pedantic $(DEFS)

all: ispalindrom

ispalindrom: ispalindrom.o help.o
	gcc -o ispalindrom ispalindrom.o help.o
	
ispalindrom.o: ispalindrom.c help.h
	gcc $(CFLAGS) -o ispalindrom.o ispalindrom.c
	
help.o: help.c help.h
	gcc $(CFLAGS) -o help.o help.c
	
clean: 
	rm -rf *.o ispalindrom
