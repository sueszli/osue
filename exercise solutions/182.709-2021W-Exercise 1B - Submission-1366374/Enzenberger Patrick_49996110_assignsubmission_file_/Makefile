
all: supervisor generator

supervisor: supervisor.o
	gcc -o supervisor supervisor.o -pthread -lrt

supervisor.o: supervisor.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c supervisor.c

generator: generator.o
	gcc -o generator generator.o -pthread -lrt

generator.o: generator.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c generator.c

clean: 
	rm supervisor supervisor.o generator generator.o