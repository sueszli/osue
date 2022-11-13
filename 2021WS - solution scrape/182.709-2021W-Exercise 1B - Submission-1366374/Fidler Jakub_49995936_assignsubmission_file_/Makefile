.PHONY: clean

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS = -lpthread -lrt # needed to link semaphores (-lpthread) /shared mem (-lrt)
TARGETS = supervisor generator

all: $(TARGETS)

supervisor: supervisor.o buffer.o
	$(CC) -o $@ $^ $(LDFLAGS)

generator: generator.o buffer.o
	$(CC) -o $@ $^ $(LDFLAGS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
	
clean:
	rm -rf $(TARGETS)
	rm -rf *.o