# @file Makefile
# @author Alex Ploner 12024704
# @date 17.11.2021
#
# @brief Program name: forkFFT

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS) -fdiagnostics-color=always
LDFLAGS = -lm

FORKFFT_OBJECTS = forkFFT.o

.PHONY: all clean release
all: forkFFT


forkFFT: $(FORKFFT_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o generator supervisor HW1B.tgz

# Create the archive to submit
release:
	tar -cvzf HW2.tgz Makefile *.c *.h