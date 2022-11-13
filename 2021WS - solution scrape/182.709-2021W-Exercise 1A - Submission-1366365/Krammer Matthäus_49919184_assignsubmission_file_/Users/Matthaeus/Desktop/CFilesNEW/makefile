CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
override CFLAGS += -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = myExpand.o
.PHONY: all clean
all: myExpand
myExpand: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
myExpand.o: myExpand.c 
clean:
	rm -rf myExpand.o myExpand.c