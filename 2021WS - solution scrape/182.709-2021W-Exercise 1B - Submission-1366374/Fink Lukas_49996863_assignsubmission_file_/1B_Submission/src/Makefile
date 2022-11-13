# author: Lukas Fink, 11911069
# program names: generator, supervisor

CC = gcc
DEFS = -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g
LDFLAGS = -lrt -pthread

.PHONY: all clean
all: generator supervisor

supervisor: supervisor.o algorithms.o datatypes.o circularBuffer.o
	$(CC) -g $(LDFLAGS) -o supervisor supervisor.o algorithms.o datatypes.o circularBuffer.o

generator: generator.o algorithms.o datatypes.o circularBuffer.o
	$(CC) -g $(LDFLAGS) -o generator generator.o algorithms.o datatypes.o circularBuffer.o


supervisor.o : supervisor.c algorithms.h datatypes.h circularBuffer.h
	$(CC) $(CFLAGS) -c -o supervisor.o supervisor.c

generator.o: generator.c algorithms.h datatypes.h circularBuffer.h
	$(CC) $(CFLAGS) -c -o generator.o generator.c


circularBuffer.o: circularBuffer.c circularBuffer.h datatypes.h
	$(CC) $(CFLAGS) -c -o circularBuffer.o circularBuffer.c

algorithms.o: algorithms.c algorithms.h datatypes.h
	$(CC) $(CFLAGS) -c -o algorithms.o algorithms.c

datatypes.o: datatypes.c datatypes.h
	$(CC) $(CFLAGS) -c -o datatypes.o datatypes.c

clean:
	rm -rf *.o generator supervisor
