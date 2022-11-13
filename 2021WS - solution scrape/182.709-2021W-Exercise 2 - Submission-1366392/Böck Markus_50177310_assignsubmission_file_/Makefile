# forksort
# Markus Böck 12020632 22.11.21

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all
all: forksort

.PHONY: clean
clean:
	rm -rf *.o forksort

%.o:	%.c
	$(CC) $(CFLAGS) -o $@ -c $<

forksort:	main.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
