CC 	= gcc
DEFS	= -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS	= -std=c99 -pedantic -Wall -g $(DEFS)

all:	myexpand

mygrep: myexpand.o
	$(CC) -o myexpand myexpand.o

mygrep.o: myexpand.c myexpand.h
	$(CC) $(CFLAGS) -c -o myexpand.o myexpand.c

clean:
	rm -rf myexpand.o myexpand
