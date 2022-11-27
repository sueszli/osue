mydiff: mydiff.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mydiff.c
	gcc -o mydiff mydiff.c

clean:
	rm -rf *.o mydiff

all: mydiff