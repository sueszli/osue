.PHONY: all clean


# compiler version (gcc-12 not available in OSUE lab)
GCC = gcc-12


# compile flags (see: https://gcc.gnu.org/onlinedocs/gcc-4.3.2/gcc/Warning-Options.html)
wflags = -Wall -Wextra -Wwrite-strings -Wconversion
defs = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
libraries = -lm

DEFAULT = -std=c99 -pedantic -Wall -g $(defs) 
VERBOSE = -std=c99 -pedantic -Wall -g3 -DOSUETREE -DDEBUG $(wflags)


# building executables
all: intmul

intmul: intmul.o
	$(GCC) -o intmul intmul.o $(libraries)

intmul.o: intmul.c
	$(GCC) $(VERBOSE) -c -o intmul.o intmul.c


# cleaning up
clean:
	rm -rf *.o intmul
