# author: Péter Békési
# program name: mydiff
# date: 2020-11-06

all: mydiff

COMPILE_FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
DEBUG_COMPILE = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -DDEBUG

mydiff: mydiff.o
	gcc -o $@ $<

mydiff.o: mydiff.c
	gcc $(COMPILE_FLAGS) -c $<

clean:
	rm -f mydiff mydiff.o
