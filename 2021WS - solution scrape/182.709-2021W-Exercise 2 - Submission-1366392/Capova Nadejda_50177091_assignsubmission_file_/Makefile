
 # @file forksort.c  
 # @author Nadejda Capova 11923550
 # @date 11.12.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: forksort

forksort : forksort.o 
	$(CC) $(CFLAGS) -o forksort forksort.o -lm

forksort.o :forksort.c
	$(CC) $(CFLAGS) -c -o  forksort.o forksort.c

clean:
	rm -rf *.o forksort