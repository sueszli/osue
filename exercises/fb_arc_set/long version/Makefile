.PHONY: all clean


# compiler version (gcc-12 not available in OSUE lab)
GCC = gcc-12


# compile flags (see: https://gcc.gnu.org/onlinedocs/gcc-4.3.2/gcc/Warning-Options.html)
wflags = -Wall -Wextra -Wwrite-strings -Wconversion
defs = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
libraries = -lpthread -pthread -lrt

DEFAULT = -std=c99 -pedantic -Wall -g $(defs)
VERBOSE = -std=c99 -pedantic -Wall -g3 -DDEBUG $(wflags) $(defs)


# building ELF-executables
all: supervisor generator

supervisor: supervisor.o
	$(GCC) -o supervisor supervisor.o $(libraries)

generator: generator.o
	$(GCC) -o generator generator.o $(libraries)

supervisor.o: supervisor.c common.h
	$(GCC) $(VERBOSE) -c -o supervisor.o supervisor.c

generator.o: generator.c common.h
	$(GCC) $(VERBOSE) -c -o generator.o generator.c


# clean up
clean:
	rm -rf *.o supervisor generator
