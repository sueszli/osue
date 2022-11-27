# @author Oliver Mayer, MatNr: 12023147
# @date 2021-11-12
#
# @brief Generates the executables for intmul

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -pthread -lrt

OBJECTS_COMMON = revstr.o unrestrictedint.o
OBJECTS_MAIN= main.o $(OBJECTS_COMMON)

.PHONY: all clean
all: intmul

intmul: $(OBJECTS_MAIN)
	$(CC) $(LDFLAGS) -o $@ $^ -lm

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

revstr.o: revstr.c revstr.h
	$(CC) $(CFLAGS) -c -o $@ $<

unrestrictedint.o: unrestrictedint.c unrestrictedint.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o intmul
