# @author Filip Markovic
# @programname forksort

CC= -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: forksort forksort.o 

forksort: forksort.o
	gcc -o forksort forksort.o

forksort.o: forksort.c
	gcc $(CC) -c forksort.c

clean: 
	rm -rf forksort.o forksort