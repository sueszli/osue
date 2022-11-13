CC=gcc
DEFS = D DEFAULT SOURCE D BSD SOURCE D SVID SOURCE D POSIX C SOURCE=200809L
CFLAGS = std=c99 pedantic Wall g $(DEFS)

OBJECTS = ispalindrom.o

.PHONY: all clean
all: ispalindrom

ispalindrom: ispalindrom.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -rf *.o ispalindrom
