#author: Alexander Ripar <e12022494@student.tuwien.ac.at>
#program name: mygrep

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = main.o init.o search.o

.PHONY: all clean
all: mygrep

mygrep: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c init.h search.h
init.o: init.c init.h
search.o: search.c search.h

clean:
	rm -rf *.o mygrep