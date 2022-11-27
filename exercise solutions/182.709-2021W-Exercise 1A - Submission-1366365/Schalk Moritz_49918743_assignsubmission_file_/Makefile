# myexpand
# Author: Mritz Schalk (52005233)
# Date: 25.10.2021

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
OBJ = myexpand.o

.PHONY: all clean

all: myexpand

debug: CFLAGS += -DDEBUG
debug: myexpand

myexpand: $(OBJ)
	$(CC) -o $@ $^

%.o: $.c
	$(CC) $(CFLAGS) -o $@ $<

clean: 
	rm -f myexpand *.o
