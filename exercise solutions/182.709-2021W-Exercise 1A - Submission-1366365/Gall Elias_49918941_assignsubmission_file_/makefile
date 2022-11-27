# Author: 		Elias GALL - 12019857
# Program name: ispalindrom

all: ispalindrom

ispalindrom: ispalindrom.o checkpalindrom.o
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -o ispalindrom ispalindrom.o checkpalindrom.o 

checkpalindrom.o: checkpalindrom.c ispalindrom.c checkpalindrom.h
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o checkpalindrom.o checkpalindrom.c

ispalindrom.o: ispalindrom.c ispalindrom.h checkpalindrom.h
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o ispalindrom.o ispalindrom.c

clean: ispalindrom.c
	rm ispalindrom ispalindrom.o checkpalindrom.o