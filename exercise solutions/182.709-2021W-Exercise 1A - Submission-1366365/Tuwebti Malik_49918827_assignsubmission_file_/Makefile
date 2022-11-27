# @author Malik Tuwebti <e11927172@student.tuwien.ac.at>
# @file Makefile
# @date 07/11/21
# @brief Makefile for the myexpand module


CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS =
OBJECTS = myexpand.o

.PHONY : all clean

all: myexpand

myexpand : $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o myexpand
