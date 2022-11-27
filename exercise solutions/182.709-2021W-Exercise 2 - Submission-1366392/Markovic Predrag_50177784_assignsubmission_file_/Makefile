
# @author: Markovic Predrag
# program: forksort

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
FLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

all: forksort
forksort: forksort.o helper_functions.o
	$(CC) -o forksort forksort.o helper_functions.o
	
forksort.o: forksort.c
	$(CC) $(FLAGS) -c -o forksort.o forksort.c


helper_functions.o: helper_functions.c
	$(CC) $(FLAGS) -c -o helper_functions.o helper_functions.c

clean: 
	rm -rf *.o
