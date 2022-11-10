# @file Makefile
# @author Yahya Jabary <mailto: yahya.jabary@tuwien.ac.at>
# @date 01.11.2022
# 
# @brief Makefile with multiple compile options to choose from.


.PHONY: all clean


# compiler version (gcc-12 not available in OSUE lab)
GCC = gcc-12


# compile flags
defs = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
libraries = -lpthread -lrt

DEFAULT = -std=c99 -pedantic -Wall -g $(defs)
LESS_VERBOSE = -std=c99 -pedantic -Wall -g -DDEBUG $(defs)
VERBOSE = -std=c99 -pedantic -Wall -Wextra -g3 -DDEBUG $(defs)


# building ELF-executables
all: supervisor generator

supervisor: supervisor.o
	$(GCC) -o $@ $^ $(libraries)

generator: generator.o
	$(GCC) -o $@ $^ $(libraries)

%.o: %.c
	$(GCC) $(LESS_VERBOSE) -c -o $@ $<


# dependencies for object files
supervisor.o: supervisor.c common.h
generator.o: generator.c common.h


# clean up
clean:
	rm -rf *.o supervisor generator
