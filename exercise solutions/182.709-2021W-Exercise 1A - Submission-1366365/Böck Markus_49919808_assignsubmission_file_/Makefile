# myexpand
# Author: Markus Böck 12020632

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -DDEFAULT_SOURCE -DBSD_SOURCE -DSVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS=

.PHONY: all
all:    myexpand

myexpand:   main.o
	$(CC) main.o -o myexpand $(LDFLAGS) $(CFLAGS)
	
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o
	
.PHONY: clean
clean: 
	rm -f myexpand main.o
