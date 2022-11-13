#@Author: Valentin Hrusa 11808205

.PHONY: clean

all: mygrep

mygrep: mygrep.o
	gcc -o mygrep mygrep.o

mygrep.o: mygrep.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mygrep.c

clean:
	rm mygrep.o mygrep