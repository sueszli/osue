# @file Makefile
# @author Jakob Frank (11837319)
# @date 09/12/2021

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
LFLAGS = -lm

all: cpair

cpair: cpair.o
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o *.h.gch cpair