#author: 	Johannes Fellner
#date: 		20211114
#mat.nr.: 	01325043
#program: 	myexpand

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTFILES = myexpand.o

.PHONY: all clean

all: myexpand

myexpand: 	$(OBJECTFILES)
	  	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

myexpand.o: myexpand.c

clean:
	rm -f $(OBJECTFILES) myexpand
