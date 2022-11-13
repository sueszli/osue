#Author: Michael Meiseneder, 51907333, <michael.meiseneder@student.tuwien.ac.at>
#Program Name: forkFFT, forkFFT_output_to_tree

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g
LFLAGS = -lm
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

.PHONY: all clear

all: forkFFT forkFFT_output_to_tree

functions.o: functions.c functions.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<

forkFFT: forkFFT.c forkFFT.h functions.o
	$(CC) $(CFLAGS) $(DEFS) $(LFLAGS) -o $@ $< functions.o

forkFFT_output_to_tree: forkFFT_output_to_tree.c forkFFT.h functions.o
	$(CC) $(CFLAGS) $(DEFS) $(LFLAGS) -o $@ $< functions.o

clean:
	rm -rf *.o forkFFT forkFFT_output_to_tree assignment_2_51907333.tgz

assignment: clean forkFFT.c forkFFT_output_to_tree.c functions.c forkFFT.h functions.h Makefile
	tar -cvzf assignment_2_51907333.tgz ./*
