# @ file makefile \
@author Florin-Elis Buju 12024755 \
@date 11.11.2021 \
@brief makefile for 1B with programs generator and supervisor

CC = gcc

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)


.PHONY: clean all

all: supervisor generator

generator: generator.o
	$(CC) $(CFLAGS) -o $@ $^ -lrt -lpthread

supervisor: supervisor.o
	$(CC) $(CFLAGS) -o $@ $^ -lrt -lpthread
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 	
	rm -f generator
	rm -f supervisor
	rm -f *.o
	
run:
	./generator 0-1
