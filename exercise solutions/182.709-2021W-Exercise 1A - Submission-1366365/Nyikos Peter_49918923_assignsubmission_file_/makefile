# author: Peter Nyikos
# date: 13/11/2021
# program name: mydiff

all: mydiff

clean:
	rm -rf *.o mydiff

mydiff: mydiff.o
	$ gcc -o mydiff mydiff.o

mydiff.o: mydiff.c
	$ gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mydiff.c

	