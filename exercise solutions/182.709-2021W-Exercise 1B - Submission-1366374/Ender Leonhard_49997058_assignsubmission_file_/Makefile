# Makefile for exercise 1b
# author Leonhard Ender (12027408)
# date 14.11.2021

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = mycompress.o

all: supervisor generator

supervisor: supervisor.o circ_buf.o
	$(CC) -o  $@ $^ -lrt -pthread

generator: generator.o circ_buf.o
	$(CC) -o $@ $^  -lrt -pthread

%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

circ_buf.o: src/circ_buf.c src/circ_buf.h
supervisor.o: src/supervisor.c src/circ_buf.h
generator.o: src/supervisor.c src/circ_buf.h

clean:
	rm -rf *.o supervisor generator
