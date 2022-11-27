CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: clean

all: supervisor generator

generator: generator.o
			gcc -o generator generator.o -lrt -lpthread

supervisor: supervisor.o
			gcc -o supervisor supervisor.o -lrt -lpthread

generator.o: generator.c structDefs.h
			gcc $(CFLAGS) -c generator.c

supervisor.o: supervisor.c structDefs.h
			gcc $(CFLAGS) -c supervisor.c

clean:
		rm *.o supervisor generator
