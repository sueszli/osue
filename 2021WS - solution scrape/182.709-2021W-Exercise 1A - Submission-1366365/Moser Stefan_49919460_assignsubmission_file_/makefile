# @file makefile
# @author Stefan Moser 12025955
# @date 9.11.2021
# @makefile compiles and links all programms

CO=-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SCOURCE -D_POSIX_C_COURSE=200809L -g

all: mygrep

mygrep: main.o readArgs.o smgrep.o
	gcc -o mygrep main.o readArgs.o smgrep.o

main.o: main.c readArgs.h smgrep.h
	gcc $(CO) -c -o main.o main.c

readArgs.o: readArgs.c readArgs.h
	gcc $(CO) -c -o readArgs.o readArgs.c

smgrep.o: smgrep.c smgrep.h
	gcc $(CO) -c -o smgrep.o smgrep.c

clean:
	rm -rf *.o *.out mygrep
