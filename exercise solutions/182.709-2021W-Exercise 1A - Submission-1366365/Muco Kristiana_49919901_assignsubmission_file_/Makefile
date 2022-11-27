#Author 01528577 Kristiana Muco
#Program name: ispalindrom

CC= gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LFLAGS = -lrt -pthread

CFILES = ispalindrom.c
PGMNAME = ispalindrom
OBJ = ispalindrom.o

all: $(PGMNAME)


ispalindrom: ispalindrom.o
	$(CC) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ispalindrom.o: ispalindrom.c

clean:
	rm -f $(PGMNAME) $(OBJ)
	rm -r doc/html
	rm -r doc/latex

doc:
	doxygen ./doc/Doxyfile

.PHONY: clean all doc

