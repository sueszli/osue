all: myexpand

myexpand: myexpand.o
	gcc -o myexpand myexpand.o

myexpand.o: myexpand.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c myexpand.c

clean:
	rm -rf *.o myexpand