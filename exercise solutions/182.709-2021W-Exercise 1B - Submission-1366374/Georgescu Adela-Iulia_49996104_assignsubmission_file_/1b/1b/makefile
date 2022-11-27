# author: Adela-Iulia Georgescu, 11810894
# programnames: generator, supervisor

.PHONY: clean

all: generator supervisor 

generator: generator.o
	gcc -o generator generator.o -lrt -pthread

generator.o: generator.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -lrt generator.c

supervisor: supervisor.o
	gcc -o supervisor supervisor.o -lrt -pthread

supervisor.o: supervisor.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -lrt supervisor.c

clean:
	rm generator generator.o supervisor supervisor.o
