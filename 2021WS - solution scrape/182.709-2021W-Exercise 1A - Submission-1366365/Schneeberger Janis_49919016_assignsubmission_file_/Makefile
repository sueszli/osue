CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_SOURCE=200809L -g

.PHONY: all clean

all : mydiff

mydiff : mydiff.o
	gcc -o $@ $^

%.o : %.c
	gcc ${CFLAGS} -c -o $@ $<

clean :
	rm -f mydiff *.o
