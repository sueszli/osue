#@author: Oliver Theimer 51840817
#@date: 08.11.2021
#@program name: ispalindrom

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS = -pthread -lrt

.PHONY: all clean

all: ispalindrom

ispalindrom: ispalindrom.o
	gcc -o ispalindrom ispalindrom.o $(LDFLAGS)

ispalindrom.o: ispalindrom.c
	gcc -c ispalindrom.c $(CFLAGS)

clean: 
	rm -rf *.o ispalindrom