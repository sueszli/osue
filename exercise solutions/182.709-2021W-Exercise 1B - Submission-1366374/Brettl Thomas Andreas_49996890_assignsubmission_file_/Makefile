all: generator supervisor

generator: generator.o graph.o
	gcc -o generator generator.o graph.o -lpthread -lrt

generator.o: generator.c graph.h
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c generator.c graph.h

supervisor: supervisor.o graph.o
	gcc -o supervisor supervisor.o graph.o -lpthread -lrt

supervisor.o: supervisor.c graph.h
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c supervisor.c graph.h

graph.o: graph.c graph.h
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c graph.c graph.h

clean:
	rm -rf *.o generator
