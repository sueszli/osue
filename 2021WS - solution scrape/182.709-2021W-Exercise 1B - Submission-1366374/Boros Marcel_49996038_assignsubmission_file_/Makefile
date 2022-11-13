DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -c -std=c99 -pedantic $(DEFS)

all: generator supervisor

generator: generator.o 
	gcc -o generator generator.o -lpthread -lrt

generator.o: generator.c generator.h
	gcc $(CFLAGS) -o generator.o generator.c 
	
supervisor: supervisor.o
	gcc -o supervisor supervisor.o -lpthread -lrt

supervisor.o: supervisor.c supervisor.h
	gcc $(CFLAGS) -o supervisor.o supervisor.c 
	
clean:
	rm -rf *.o generator supervisor
