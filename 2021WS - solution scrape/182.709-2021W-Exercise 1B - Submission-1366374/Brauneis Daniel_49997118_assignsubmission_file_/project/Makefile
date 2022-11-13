# @author daniel brauneis 12021357
# @programname ispalindrom
CC = gcc
C_FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -pthread -lrt

.PHONY: clean all documentation

all: supervisor generator

supervisor: *.c
	$(CC) $(C_FLAGS) -o supervisor.o supervisor.c

generator: *.c
	$(CC) $(C_FLAGS) -o generator.o generator.c

documentation:
	doxygen Doxyfile

runsup:
	./supervisor.o

rungen:
	./generator.o 0-1 1-2 1-3 1-4 2-4 3-6

clean:
	rm -f *.o
	rm -f -r latex html
