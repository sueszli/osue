all: mygrep clean

mygrep: mygrep.o
	gcc -std=c99 -o mygrep mygrep.o

mygrep.o: mygrep.c mygrep.h
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mygrep.c

clean: 
	rm -rf *.o