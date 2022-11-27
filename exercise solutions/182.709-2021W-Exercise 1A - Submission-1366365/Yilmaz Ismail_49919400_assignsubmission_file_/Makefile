CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic -pthread $(DEFS)
OBJECTS = main.o 

.PHONY: all clean
all: mydiff

mydiff: $(OBJECTS)	
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c 
clean:
	rm -rf *.o main mydiff
