# @file Makefile
# @author Alex Ploner 12024704
# @date 23.10.2021
#
# @brief Program name: mygrep

CC ?= gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
DEBUG_FLAGS = -Werror
TARGET = mygrep
OBJS = mygrep.o

.PHONY: all clean
all: mygrep

mygrep: $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) *.o $(TARGET)