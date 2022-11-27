# @file Makefile
# @author George Tokmaji <e11908523@student.tuwien.ac.at>
# @date 14.11.2021
#
# @brief Program name: myexpand

CC ?= gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g $(DEBUG_FLAGS)
DEBUG_FLAGS = -Werror=implicit-function-declaration -Werror

.PHONY: all clean
all: myexpand

myexpand: myexpand.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) *.o myexpand
