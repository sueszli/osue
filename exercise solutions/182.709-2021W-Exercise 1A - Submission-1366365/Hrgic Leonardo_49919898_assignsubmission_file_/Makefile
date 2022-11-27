CFLAGS = --std=c99 --pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean
all: mygrep

mygrep: mygrep.o
	gcc -o mygrep mygrep.o

mygrep.o: mygrep.c
	gcc $(CFLAGS) -c -o mygrep.o mygrep.c

clean:
		rm -rf mygrep.o mygrep
