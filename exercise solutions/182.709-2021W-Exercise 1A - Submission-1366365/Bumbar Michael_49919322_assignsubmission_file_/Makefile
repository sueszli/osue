FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

OBJECTS = main.o

.PHONY: all clean

all: ispalindrom

ispalindrom: $(OBJECTS)
	gcc $(FLAGS) -o $@ $^

%.o: %.c
	gcc $(FLAGS) -c -o $@ $<

main.o: main.c

clean:
	rm -rf *.o ispalindrom
