# Author: Moritz Schalk (52005233)
# Date: 25 Nov 20201

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_POSIX_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
LFLAGS = -lm
OBJ = main.o complex.o dynarray.o
ONAME = forkFFT

.PHONY: all clean

all: $(OBJ)
	$(CC) $(LFLAGS) -o $(ONAME) $(OBJ)

tree: CFLAGS += -DTREE
tree: ONAME = forkFFTtree
tree: all

main.o: main.c
	$(CC) $(CFLAGS) -o $@ $<

complex.o: complex.c complex.h 
	$(CC) $(CFLAGS) -o $@ $<

dynarray.o: dynarray.c dynarray.h 
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(OBJ) forkFFT forkFFTtree
