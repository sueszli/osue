#author: Jasmine Haider 11943664
#exercise 1a mygrep

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -c -Wextra -std=c99 -pedantic $(DEFS)

OBJECTS = main.o mygrep.o

.PHONY: all clean
all: mygrep

mygrep: mygrep.o main.o
	gcc $(LDFLAGS) -o mygrep mygrep.o main.o

main.o: main.c mygrep.h
	gcc $(CFLAGS) -c -o main.o main.c

mygrep.o: mygrep.c mygrep.h
	gcc $(CFLAGS) -c -o mygrep.o mygrep.c

clean:
	rm -rf *.o mygrep
