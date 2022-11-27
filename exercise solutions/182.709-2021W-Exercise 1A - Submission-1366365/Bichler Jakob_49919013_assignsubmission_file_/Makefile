# @author Jakob Bichler (11806473)
# @date 2021-11-10
# @brief Makefile OSUE tasks

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -std=c99 -pedantic $(DEFS) -g

# add filenames
OBJECTS =  myexpand

.PHONY: all clean 
all: $(OBJECTS)

$(OBJECTS): % :%.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o $(OBJECTS)
