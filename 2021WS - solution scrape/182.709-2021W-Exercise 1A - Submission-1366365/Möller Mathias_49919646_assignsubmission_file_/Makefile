# Author: Mathias Möler
# Program Name: myexpand

all: myexpand

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

myexpand: myexpand.o
	$(CC) -o myexpand myexpand.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf myexpand myexpand.o