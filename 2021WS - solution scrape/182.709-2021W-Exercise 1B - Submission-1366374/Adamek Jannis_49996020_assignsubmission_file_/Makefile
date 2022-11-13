# @file Makefile
# @author Jannis Adamek (11809490)
# @date 2021-11-13
#
# @brief Makefile for assignment 1B.

CC      := gcc 
FLAGS   := -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LIBS    := -lm -lpthread -lrt

# Compile both the supervisior and the generator
all: supervisior generator

generator: generator.o graph.o
	$(CC) ${FLAGS} -o $@ $^ ${LIBS}
	chmod +x generator

supervisior: supervisor.o graph.o
	$(CC) ${FLAGS} -o $@ $^ ${LIBS}
	chmod +x supervisior

%.o: %.c
	$(CC) ${FLAGS} -c -o $@ $<

# Remove the executable.
clean:
	find -name '*.o' -delete
	find -name generator -delete
	find -name supervisior -delete
