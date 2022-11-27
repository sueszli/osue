# @file Makefile
# @author Marc Reiterer 12020640 <e12020640@student.tuwien.ac.at>
# @date 19.11.2020
# @brief Makefile for exercise 1A - mydiff

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean

all: mydiff

mydiff: mydiff.o
	$(CC) -o mydiff mydiff.o

mydiff.o: src/mydiff.c
	$(CC) $(CFLAGS) -c src/mydiff.c


clean: 
	rm -rf *.o mydiff


