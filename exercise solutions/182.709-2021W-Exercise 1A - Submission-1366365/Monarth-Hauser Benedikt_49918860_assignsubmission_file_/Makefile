# @file Makefile
# @author Benedikt Monarth-Hauser, 01501767 <benedikt.monarth-hauser@tuwien.ac.at>
# @date 14.11.2021
#
# @brief Makefile for isapalindrome.

all: isapalindrome

isapalindrome: isapalindrome.o
	gcc -o isapalindrome isapalindrome.o

isapalindrome.o: isapalindrome.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o isapalindrome.o isapalindrome.c

clean:
	rm *.o	isapalindrome