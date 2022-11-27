# @file Makefile
# @author Jakob Frank(11837319)
# @date 12/11/2021


CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LFLAGS = -lpthread -lrt


all: supervisor generator

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor: Supervisor/supervisor.o shared_Memory.o circular_buffer.o
	$(CC) -o $@ $^ $(LFLAGS) 

generator: Generator/generator.o shared_Memory.o circular_buffer.o
	$(CC) -o $@ $^ $(LFLAGS)

clean:
	rm -rf *.o Generator/*.o Supervisor/*.o *.h.gch generator supervisor
