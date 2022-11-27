#@file makefile
#@author Nicholas Harisch 11705361<harischnicholas@gmail.com>
#@date 12.11.2021
#@brief makefile
CC = gcc
DEFS = 	-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS = -pthread -lrt

OBJECTS = generator.o supervisor.o
.PHONY: all clean
all: generator supervisor

generator: generator.o
		$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
supervisor: supervisor.o
		$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
%.o: %c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -rf *o supervisor generator