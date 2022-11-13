# 
# @file Makefile
# @author Kieffer Joé <e11814254@student.tuwien.ac.at>
# @date 08.11.2021
#

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

OBJECTS = supervisor.o generator.o

.PHONY: all clean
all: supervisor generator

%: %.o
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

#supervisor: supervisor.o
#	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

#generator: generator.o
#	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

supervisor: supervisor.o
generator: generator.o

clean:
	rm -rf *.o supervisor generator