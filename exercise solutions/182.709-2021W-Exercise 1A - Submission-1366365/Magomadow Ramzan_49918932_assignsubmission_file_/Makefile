# Ramzan Magomadow 12024698
# mycompress

all: mycompress

mycompress: mycompress.o
	gcc -o mycompress mycompress.o $(LINKERFLAGS)

mycompress.o: mycompress.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c $<

clean:
	rm -rf mycompress.o mycompress
