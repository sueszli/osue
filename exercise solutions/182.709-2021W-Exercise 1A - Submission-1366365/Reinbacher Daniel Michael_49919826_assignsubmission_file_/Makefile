# author: Daniel Reinbacher (01614435)
# programe name: mygrep

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = main.o mygrep.o errorhandler.o

.PHONY: all clean
all: mygrep

mygrep: $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c mygrep.h errorhandler.h
mygrep.o: mygrep.c mygrep.h errorhandler.h
errorhandler.o: errorhandler.c errorhandler.h

clean:
	rm -rf *.o mygrep