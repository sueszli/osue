# myexpand makefile
# Author Tobias Kutscher e1634063.tuwien.ac.at
# date 14.11.202

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D__BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -std=c99 -pedantic $(DEFS)

OBJECTS = myexpand.o

.PHONY: all clean
all: myexpand

%.o:
	$(CC) $(CFLAGS) -g -c -o $@ $<

myexpand.o: myexpand.c myexpand.h
myexpand: myexpand.o

clean:
	rm -rf *.o
