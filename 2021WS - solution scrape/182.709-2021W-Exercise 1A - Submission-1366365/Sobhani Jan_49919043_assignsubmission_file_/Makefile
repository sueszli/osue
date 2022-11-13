# @file Makefile
# @author Jan Sobhani <e0411385@student.tuwien.ac.at>
# @date 09.11.2021
# @program: mygrep
# @brief: Makefile-file for streamlining the compilation/linking.

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
CFLAGS = -std=c99 -pedantic -Wall $(DEFS)

OBJECTS = mygrep.o

.PHONY: all clean

all = mygrep

mygrep:$(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mygrep.o: mygrep.c

clean:
	rm -rf *.o mygrep
