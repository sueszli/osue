#Author:	Christoph Kraus 117768	
#Created: 	11.11.2021
#Programs: 	mygrep

CC	= gcc	
CFLAGS 	= -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean
all: mygrep

mygrep: mygrep.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm *.o mygrep
