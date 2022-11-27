# 
# @file Makefile
# @author Kieffer Joé <e11814254@student.tuwien.ac.at>
# @date 10.12.2021
#

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

OBJECTS = forkFFT.o

.PHONY: all clean
all: forkFFT

%: %.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

forkFFT: forkFFT.o

clean:
	rm -rf *.o forkFFT