LINKERFLAGS = -lrt -pthread

all: supervisor generator

supervisor: supervisor.o shared_graph.o
	gcc -o $@ $^ $(LINKERFLAGS)

generator: generator.o shared_graph.o
	gcc -o $@ $^ $(LINKERFLAGS)

%.o: %.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o $@ $<

clean:
	rm -rf *.o supervisor generator