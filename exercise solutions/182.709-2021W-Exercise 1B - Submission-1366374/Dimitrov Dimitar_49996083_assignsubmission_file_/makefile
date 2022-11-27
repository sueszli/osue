CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
LIBS = -lrt -lpthread

all: generator.c supervisor.c
	gcc $(CFLAGS) generator.c supervisor.c $(LIBS)
	gcc generator.o -o generator $(LIBS)
	gcc supervisor.o -o supervisor $(LIBS)

generator: generator.c
	gcc $(CFLAGS) generator.c
	gcc generator.o -o generator $(LIBS)

supervisor: supervisor.c
	gcc $(CFLAGS) supervisor.c
	gcc supervisor.o -o supervisor $(LIBS)

clean:
	$(RM) generator
	$(RM) supervisor
	$(RM) *.o
	$(RM) *.tgz

package:
	tar -cvzf 3coloring.tgz generator.c supervisor.c makefile

