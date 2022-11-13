# Author: Michael Zischg (12024010)
# Date: 07.11.2021 

.PHONY: all clean

all: forksort

forksort: forksort.o
	gcc -o forksort forksort.o

forksort.o: forksort.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c forksort.c

clean: 
	rm -f *.o forksort
