all: mydiff

compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g


mydiff: mydiff.o
	gcc -o mydiff mydiff.o

mydiff.o: mydiff.c
	gcc $(compile_flags) -c -o mydiff.o mydiff.c

clean:
	rm -rf mydiff.o mydiff