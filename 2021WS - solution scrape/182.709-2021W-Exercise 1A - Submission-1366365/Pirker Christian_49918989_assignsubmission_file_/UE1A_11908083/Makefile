#myexpand

CC = gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: myexpand

myexpand: myexpand.o 
	$(CC) -o myexpand myexpand.c

myexpand.o: myexpand.c
	$(CC) -g -o myexpand.o myexpand.c

clean:
	rm -rf *.o myexpand
	@echo "Cleaning done"