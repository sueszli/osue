# @ file makefile \
@author Florin-Elis Buju 12024755 \
@date 04.11.2021 \
@brief makefile for 1A with program mygrep

CC = gcc

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: clean all

all: mygrep

mygrep: mygrep.o
	$(CC) $(CFLAGS) -o $@ $^
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean: 	
	rm -f mygrep
	rm -f *.o
	
run:
	./mygrep test
