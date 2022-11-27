# Mario Fentler 12025969
# feedbackArcSet

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt

all: generator supervisor

generator: generator.o
	gcc $(CFLAGS) -o generator $< $(LDFLAGS)

generator.o: generator.c
	gcc $(CFLAGS) -g -c -o $@ $<

supervisor: supervisor.o
	gcc $(CFLAGS) -o supervisor $< $(LDFLAGS)

supervisor.o: supervisor.c
	gcc $(CFLAGS) -g -c -o $@ $<

clean:
	rm -rf *.o feedbackArcSet
