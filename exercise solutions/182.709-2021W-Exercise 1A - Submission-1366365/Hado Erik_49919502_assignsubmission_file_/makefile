#file makefile
#author Erik Hado <e11811852@student.tuwien.ac.at>


CC=gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
CFLAGS = -std=c99 -pedantic -Wall $(DEFS)
LDFLAGS = -o

.PHONY: all clean
all: myexpand

myexpand: myexpand.o
	$(CC) $(LDFLAGS) myexpand myexpand.o

myexpand.o: myexpand.c
	$(CC) $(CFLAGS) -c -o myexpand.o myexpand.c

clean:
	rm -rf *.o *.out myexpand