# @file generator.c, supervisor.c, utility.h & utility.c
 # @author Nadejda Capova 11923550
 # @date 26.10.2020


CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: generator supervisor

#executes all the files

generator: generator.c
	$(CC) $(CFLAGS) -o generator generator.c -pthread -lrt

supervisor: supervisor.c
	$(CC) $(CFLAGS) -o supervisor supervisor.c -pthread -lrt

clean:
	rm -rf *.o generator supervisor