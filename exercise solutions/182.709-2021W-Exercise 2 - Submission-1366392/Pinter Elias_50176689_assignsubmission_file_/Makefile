# name: Elias Pinter
# matriculation number: 12023962
# programm-name: forksort 

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = forksort.o

.PHONY: all clean
all: forksort

forksort: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forksort.o: forksort.c


clean:
	rm -rf *.o forksort