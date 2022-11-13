#/**
#* @program name myexpand
#* @author Georges Kalmar 09803393
#*/

CC=gcc
OBJECTS=myexpand.o main.o
DEFS=-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)

.PHONY:all clean

all:myexpand

myexpand: $(OBJECTS)
	$(CC) -o $@ $^
	
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
myexpand.o: myexpand.c myexpand.h
main.o: main.c myexpand.h

clean:
	rm -rf *.o myexpand