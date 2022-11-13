
CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mygrep.o mygreplines.o


.PHONY: all clean
all: mygrep

mygrep: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -lpthread -lrt

mygrep.o: mygrep.c mygrep.h
mygreplines.o : mygreplines.c mygrep.h


clean:
	rm -rf *.o mygrep
