#author: Manuel Hochreiter e0671428@student.tuwien.ac.at
#program_name: generator supervisor

# Unfortunately I discovered last minute that my generator is only able to read single digit vertices. Fixing this issue I would have used strtol for getting the integers from the char input,
# and then write the edges back in the char array that gets written into the buffer. As there was not enough time left I did not want to risk ending up with a program that does not compile, 
# hence the limit to single digit vertices.

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -pthread

OBJECT1 = generator.o 
OBJECT2 = supervisor.o

.PHONY: all clean
all: generator supervisor

generator: $(OBJECT1)
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -lm

supervisor: $(OBJECT2)
	$(CC) $(LDFLAGS) -o $@ $^ -lrt

%.o: %.c header.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -rf *.o generator supervisor