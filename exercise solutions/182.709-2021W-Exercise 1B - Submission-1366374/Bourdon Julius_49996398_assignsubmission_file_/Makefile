 # @file Makefile
 # @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 # @date 14.11.2021
 #
 # @brief Makefile for easier compilation and linking as well 
 #        as removing of unused object-files and executables 
 # 

CC = gcc
DEFS    = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS  = -std=c99 -pedantic -Wall $(DEFS) -g

.PHONY: all clean

all: generator supervisor
clean: 
	rm -rf *.o generator supervisor
clean_without_exe:
	rm -rf *.o


generator: generator.o fb_arc_set.o
	$(CC) $(CFLAGS) -pthread -lrt -o generator generator.o fb_arc_set.o

generator.o: generator.c fb_arc_set.h
	$(CC) $(CFLAGS) -c -o generator.o generator.c

supervisor: supervisor.o fb_arc_set.o
	$(CC) $(CFLAGS) -pthread -lrt -o supervisor supervisor.o fb_arc_set.o

supervisor.o: supervisor.c fb_arc_set.h
	$(CC) $(CFLAGS) -c -o supervisor.o supervisor.c

fb_arc_set.o: fb_arc_set.c fb_arc_set.h
	$(CC) $(CFLAGS) -c -o fb_arc_set.o fb_arc_set.c



