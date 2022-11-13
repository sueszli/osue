# author: Moritz Bauer, 0649647
# creates: generator, supervisor
# date: 21.11.11

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -pthread -g -std=c99 -pedantic $(DEFS)

EXEC_NAME_GENERATOR = generator
OBJECTS_GENERATOR = shuffle.o generator.o

EXEC_NAME_SUPERVISOR = supervisor
OBJECTS_SUPERVISOR = supervisor.o

.PHONY: all clean

runAll: clean all

all: $(EXEC_NAME_GENERATOR) $(EXEC_NAME_SUPERVISOR) $(EXEC_NAME_CLI)

clean:
	rm -rf *.o $(EXEC_NAME_GENERATOR) $(EXEC_NAME_SUPERVISOR) $(EXEC_NAME_CLI)

$(EXEC_NAME_GENERATOR): $(OBJECTS_GENERATOR)
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

$(EXEC_NAME_SUPERVISOR): $(OBJECTS_SUPERVISOR)
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

archive: delete_archive
	tar -cvzf 00649647.1b.tgz Makefile *.h *.c

delete_archive:
	rm -rf *.tgz