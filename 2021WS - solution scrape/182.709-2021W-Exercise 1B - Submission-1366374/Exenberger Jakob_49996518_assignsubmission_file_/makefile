# @author: Jakob Exenberger - 11707692
# @name: 3colour

all: generator supervisor

generator: generator.o
	gcc -o generator generator.o -lpthread -lrt

supervisor: supervisor.o
	gcc -o supervisor supervisor.o -lpthread -lrt

generator.o: generator.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c generator.c 

supervisor.o: supervisor.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c supervisor.c 

clean: 
	rm -rf *.o supervisor generator