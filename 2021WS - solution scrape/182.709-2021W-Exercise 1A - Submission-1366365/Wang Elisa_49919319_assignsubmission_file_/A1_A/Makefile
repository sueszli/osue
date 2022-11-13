# @file Makefile
# @author Elisa Wang <e12023138@student.tuwien.ac.at>
# @date 13.11.2021
#
# @brief Makefile to compile and link program for mycompress
# 

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
LFLAGS = 
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mycompress.o

.PHONY: all clean
all: mycompress

mycompress: mycompress.o
	$(CC) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mycompress.o: mycompress.c

clean:
	rm -rf *.o mycompress