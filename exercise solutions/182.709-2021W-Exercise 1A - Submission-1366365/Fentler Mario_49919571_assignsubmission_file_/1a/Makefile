# Mario Fentler 12025969
# mycompress

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

all: mycompress.o
	gcc $(CFLAGS) -o mycompress $<

mycompress.o: mycompress.c mycompress.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mycompress
