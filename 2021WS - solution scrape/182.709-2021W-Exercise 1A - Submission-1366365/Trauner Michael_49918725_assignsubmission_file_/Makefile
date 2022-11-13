 # @file functions.h
 # @author Michael Trauner <e12019868@student.tuwien.ac.at>
 # @date 09.11.2021

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = main.o functions.o

.PHONY: all clean
all: aufgabe1a

aufgabe1a: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c functions.h
functions.o: functions.c functions.h

clean:
	rm -rf *.o functions