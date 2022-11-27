## @author: David Brand <e11905164@student.tuwien.ac.at>
## @program: 3coloring

CC		= gcc
DEFS	= -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS	= -std=c99 -pedantic -Wall -g $(DEFS)

./PHONY: all clean
all: supervisor generator

supervisor: supervisor.o
	gcc -o supervisor supervisor.o -pthread -lrt

supervisor.o: supervisor.c
	gcc $(CFLAGS) -c supervisor.c

generator: generator.o
	gcc -o generator generator.o -pthread -lrt

generator.o: generator.c
	gcc $(CFLAGS) -c generator.c

clean:
	rm -rf *.o supervisor generator