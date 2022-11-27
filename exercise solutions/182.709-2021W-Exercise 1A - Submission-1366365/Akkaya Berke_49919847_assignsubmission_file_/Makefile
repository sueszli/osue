# @file Makefile
# @author Berke Akkaya 11904656
# @date 14.11.2021
# program: ispalindrom



all: ispalindrom

compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

ispalindrom.o: ispalindrom.c
	gcc $(compile_flags) -c ispalindrom.c

ispalindrom: ispalindrom.o
	gcc -o ispalindrom ispalindrom.o

clean:
	rm -f ispalindrom ispalindrom.o