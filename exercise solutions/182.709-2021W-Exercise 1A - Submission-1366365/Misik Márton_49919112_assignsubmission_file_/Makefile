CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = assA.o utils.o

.PHONY: all clean
all: myprog

myprog: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

assA.o: assA.c assA.h
utils.o: utils.c utils.h

clean:
	rm -rf *.o myprog