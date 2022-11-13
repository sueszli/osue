#Patrick Holly 11807878 fb_arc_set supervisor & generator
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
AFLAGS = -lrt -lpthread
OBJECT1 = generator.o 
OBJECT2 = supervisor.o
CC=gcc

.PHONY: all clean
all:generator supervisor

generator: $(OBJECT1)
				$(CC) $(CFLAGS) -o $@ $^ $(AFLAGS)
%.o: %.c
				$(CC) $(CFLAGS) -c -o $@ $< $(AFLAGS)

supervisor: $(OBJECT2)
				$(CC) $(CFLAGS) -o $@ $^ $(AFLAGS)
%.o: %.c
				$(CC) $(CFLAGS) -c -o $@ $< $(AFLAGS)
clean:
				rm -rf *.o generator supervisor


