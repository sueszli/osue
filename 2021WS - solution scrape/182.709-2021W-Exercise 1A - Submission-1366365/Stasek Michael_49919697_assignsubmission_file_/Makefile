#@file Makefile
#@author Michael Stasek 11777727
#@date 14.11.2021

#@brief Makefile for ispalindrom


C = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

all: ispalindrom


%: %.c
		$(CC) $(CFLAGS) $< -o $@

clean:
		rm -rf *.o ispalindrom
