
 # @name Makefile
 # @author  Thomas Scharinger, MatrNr: 11777710
 # @date 12.11.2021
 # @brief This is the makefile for the program mydiff.c
 

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean

all: mydiff

mydiff.o: mydiff.c
	gcc $(CFLAGS) -c -o $@ $^

mydiff: mydiff.o
	gcc $(CFLAGS) -o $@ $^




clean:
	rm -rf	mydiff.o mydiff
