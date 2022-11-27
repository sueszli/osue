#Patrick Holly 11807878 mycompress
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
LCFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECT = mycompress.o
CC = gcc

.PHONY: all clean
all:mycompress
mycompress: $(OBJECT)
		$(CC) $(LCFLAGS) -o $@ $^
%.o: %.c
		$(CC) $(LCFLAGS) -c -o $@ $^

clean:
		rm -rf *.o mycompress

