###
#
# Author : Radovan Vukovic (01428736) <e1428736@student.tuwien.ac.at>
# Date : 13.11.2021
# Program ispalindrom.c
#
###

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean

all: ispalindrom

ispalindrom: ispalindrom.o
		$(CC) -o $@ $^

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

ispalindrom.o: ispalindrom.c

clean: 
	rm -f *.o 
	rm -f ispalindrom

