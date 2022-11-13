# author: Marvin Michlits 11731205
# program: mygrep
ALL := $(wildcard *)
ALL := $(filter-out grep.c, $(ALL))
ALL := $(filter-out grep.h, $(ALL))
ALL := $(filter-out main.c, $(ALL))
ALL := $(filter-out Makefile, $(ALL))
all: mygrep
mygrep: grep.o main.o
		gcc -o mygrep main.o grep.o
grep.o: grep.c grep.h
		gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c grep.c
main.o: main.c
		gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c main.c
clean:
		rm -rf $(ALL)
.PHONY: all clean