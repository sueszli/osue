# @programname mycompress
# @author Laurenz Gaisch <e11808218@student.tuwien.ac.at>

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

.PHONY: all
all: mycompress

mycompress: mycompress.o
	gcc -g -o mycompress mycompress.o

mycompress.o: mycompress.c
	gcc $(CFLAGS) mycompress.c

clean:
	rm -rf mycompress *.o