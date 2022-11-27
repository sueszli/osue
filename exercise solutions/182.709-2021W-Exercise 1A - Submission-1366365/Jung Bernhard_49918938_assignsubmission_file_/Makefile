CC = gcc
PN = mygrep

all:
	$(CC) -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -o $(PN) main.c

execute: all
	./$(PN) -o outPutTest.txt iNpUt 

clean:
	rm $(PN)