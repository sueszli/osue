#file Makefile
#program intmult
#author Marcel Boros <e11937377@student.tuwien.ac.at>
#date 06.12.2021

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -c -std=c99 -pedantic $(DEFS)

all: intmult

intmult: readIn.o mult.o intmult.o
	gcc -o intmult intmult.o readIn.o mult.o -lm

readIn.o: readIn.c readIn.h
	gcc $(CFLAGS) -o readIn.o readIn.c 
	
mult.o: mult.c mult.h
	gcc $(CFLAGS) -o mult.o mult.c
	
intmult.o : intmult.c
	gcc $(CFLAGS) -o intmult.o intmult.c
	
clean:
	rm -rf *.o intmult
