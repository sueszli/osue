##
# @author Michael Winter 01426162
# @brief Recursive mergesort using pipes & forking
# @date 05.12.2021

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS += -std=c99 -pedantic -Wall -g -c $(DEFS)
LDFLAGS +=

#objects to build
OBJS = forksort.o

.PHONY : all clean

all: forksort

fortsort: $(OBJS)
	gcc $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f forksort $(OBJS)

# dependencies
forksort.o: forksort.c

