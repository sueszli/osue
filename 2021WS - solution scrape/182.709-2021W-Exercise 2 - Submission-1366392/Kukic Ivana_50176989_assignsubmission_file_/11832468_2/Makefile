#Author 11832468 - Ivana Kukic
#Program name: cpair

CC= gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LFLAGS = -lm -pthread

CFILES = cpair.c
PGMNAME = cpair
OBJ = cpair.o

all: $(PGMNAME)


cpair: cpair.o
	$(CC) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

cpair.o: cpair.c 

clean:
	rm -f $(PGMNAME) $(OBJ)

.PHONY: clean all