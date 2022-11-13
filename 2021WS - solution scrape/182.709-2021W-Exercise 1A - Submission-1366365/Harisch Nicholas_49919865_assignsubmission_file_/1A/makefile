#@file makefile
#@author Nicholas Harisch 11705361<harischnicholas@gmail.com>
#@date 10.11.2021
#@brief makefile
CC = gcc
DEFS = 	-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS = -pthread -lrt

OBJECTS = mydiff.o 
.PHONY: all clean
all: mydiff

mydiff: mydiff.o
		$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
%.o: %c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -rf *o mydiff