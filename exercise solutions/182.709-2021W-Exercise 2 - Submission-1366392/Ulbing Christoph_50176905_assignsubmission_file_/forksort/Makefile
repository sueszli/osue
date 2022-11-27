# author: Christoph Ulbing 12019872
# program name: forksort


all: forksort

compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

forksort.o: forksort.c
	gcc $(compile_flags) -c forksort.c

forksort: forksort.o
	gcc -o forksort forksort.o

clean:
	rm -f forksort forksort.o