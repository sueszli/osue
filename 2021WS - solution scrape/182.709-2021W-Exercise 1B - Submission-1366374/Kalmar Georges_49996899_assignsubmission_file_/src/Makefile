#/**
#* @program name threecoloring
#* @author Georges Kalmar 09803393
#*/

CC= gcc
DEFS= -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS= -Wall -g -std=c99 -pedantic $(DEFS)
DFLAGS= -lpthread -lrt

.PHONY: all clean

all: generator supervisor

generator: main.o generator.o
	$(CC) -o $@ $^ $(DFLAGS)

supervisor: supervisor.o
	$(CC) -o $@ $< $(DFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c generator.h
generator.o: generator.c generator.h
supervisor.o: supervisor.c supervisor.h

clean:
	rm -rf *.o generator supervisor