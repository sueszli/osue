#12024014
all: mygrep.o
	gcc -o mygrep mygrep.o

mygrep.o: mygrep.c
	gcc -std=c99 -pedantic -Wall -D DEFAULT_SOURCE -D BSD_SOURCE -D SVID_SOURCE -D POSIX_C_SOURCE=200809L -g -c mygrep.c

clean: 
	rm -f mygrep.o mygrep