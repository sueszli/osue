CC = gcc

FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

OBJECTS = mygrep.o

.PHONY: all clean
all: mygrep

mygrep: $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^


*.o: *.c
	$(CC) $(FLAGS) -c -o $@ $<
	
mygrep.o: mygrep.c

clean:
	rm -rf *.o mygrep
