# @file Makefile
# @author Larissa Artlieb 
# @date 13.11.2021
# @brief Makefile for compiling the project
# @details 	This makefile defines which source files are needed for compilation.
#			It can also be used to clean up the project directory using "make clean".

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread
SUPERVISOROBJECTS = supervisor.o
GENERATOROBJECTS = generator.o
.PHONY: all clean
all: supervisor generator
supervisor: $(SUPERVISOROBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

generator: $(GENERATOROBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
supervisor.o: supervisor.c supervisor.h
generator.o: generator.c generator.h supervisor.h
clean:
	rm -rf *.o supervisor generator
