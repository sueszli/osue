# Author: Adrian Chroust <e12023146@student.tuwien.ac.at>
# Date: 12.11.2021

# Name of supervisor executable
SUPERVISOR = supervisor

# Name of generator executable
GENERATOR = generator

# Compiler flags
CC = gcc
FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LIBRARIES = -lrt -pthread
COMPILE = $(CC) $(FLAGS)

.PHONY: all clean

all: $(SUPERVISOR) $(GENERATOR)

clean:
	rm -rf src/*.o $(SUPERVISOR) $(GENERATOR)

$(SUPERVISOR): src/supervisor.o src/sharedcircularbuffer.o
	$(COMPILE) -o $@ $^ $(LIBRARIES)

$(GENERATOR): src/generator.o src/sharedcircularbuffer.o
	$(COMPILE) -o $@ $^ $(LIBRARIES)

%.o: %.c
	$(COMPILE) -c -o $@ $< $(LIBRARIES)

src/supervisor.o: src/supervisor.c src/sharedcircularbuffer.h
src/generator.o: src/generator.c src/sharedcircularbuffer.h
src/sharedcircularbuffer.o: src/sharedcircularbuffer.c src/sharedcircularbuffer.h
