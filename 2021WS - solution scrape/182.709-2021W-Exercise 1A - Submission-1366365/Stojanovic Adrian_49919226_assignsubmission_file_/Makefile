# Adrian Stojanovic 11908093
# name of program: myexpand

all: myexpand

CFLAGS= -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

myexpand: myexpand.o
	gcc -o myexpand myexpand.o

myexpand.o: myexpand.c
	gcc $(CFLAGS) -c -o myexpand.o myexpand.c

clean: 
	rm -rf myexpand myexpand.o
