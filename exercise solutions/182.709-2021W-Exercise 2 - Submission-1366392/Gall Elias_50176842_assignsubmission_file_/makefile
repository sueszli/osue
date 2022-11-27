# Author: Elias GALL - 12019857
# Program: intmul

all: intmul

intmul: intmul.o
	gcc -o intmul -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g intmul.o

intmul.o: intmul.c
	gcc -o intmul.o -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c intmul.c

clean: 
	rm intmul.o intmul