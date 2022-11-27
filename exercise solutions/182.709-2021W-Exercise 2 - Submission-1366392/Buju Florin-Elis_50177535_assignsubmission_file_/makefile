# @ file makefile \
@author Florin-Elis Buju 12024755 \
@date 4.12.2021 \
@brief makefile for program 2

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)


.PHONY: clean all

all: forkFFT

forkFFT: forkFFT.o
	$(CC) $(CFLAGS) -o $@ $^ -lm
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 	
	rm -f forkFFT
	rm -f *.o
	
run:
	./forkFFT < test.txt
