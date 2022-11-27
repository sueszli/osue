#makefile for Assignment 1A OSUE 2021 ispalindromm
#author: Sejdefa Ibisevic <e11913116@student.tuwien.ac.at>

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g 

all: ispalindrom

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf ispalindrom ispalindrom.o Assignment_1A_11913116.tgz
	
tar:
	tar -cvzf Assignment_1A_11913116.tgz Makefile *.c html latex palindrom
