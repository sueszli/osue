# Author: Philipp Gorke (e12022511)
# Created: 06.12.2021
# Programs: forksort

FLAGS = -g -c -Wall -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: forksort

forksort: forksort.o
	gcc -o forksort forksort.o

forksort.o: forksort.c 
	gcc $(FLAGS) forksort.c

clean: 
	rm -rf *.o *.out forksort		