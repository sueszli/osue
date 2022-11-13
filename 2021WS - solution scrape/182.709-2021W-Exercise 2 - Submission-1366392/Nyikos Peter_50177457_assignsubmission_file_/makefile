# author: Peter Nyikos
# date: 11/12/2021
# program name: forksort

all: forksort

clean:
	rm -rf *.o forksort

forksort: forksort.o
	$ gcc -lpthread -lrt -o forksort forksort.o

forksort.o: forksort.c
	$ gcc  -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c forksort.c