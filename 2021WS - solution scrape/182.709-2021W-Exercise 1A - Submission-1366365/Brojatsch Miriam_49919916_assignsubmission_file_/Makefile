all: ispalindrom

FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

ispalindrom.o: ispalindrom.c
	gcc $(FLAGS) -c -o ispalindrom.o ispalindrom.c

ispalindrom: ispalindrom.o
	gcc -o ispalindrom ispalindrom.o

clean:
	rm -f ispalindrom ispalindrom.o