# @file Makefile
# @author Felix Droschl 11840546
# @brief Program name: ispalindrom

all: ispalindrom

ispalindrom.o: ispalindrom.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c ispalindrom.c

ispalindrom: ispalindrom.o
	gcc ispalindrom.o -o ispalindrom
	
clean: 
	rm -f ispalindrom ispalindrom.o
