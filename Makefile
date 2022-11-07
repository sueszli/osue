# ----- compiler config (see ./run.sh) ----- 
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
ASANA = -fsanitize=address
COMPILER = gcc-12
FLAGS = $(ASANA) -std=c99 -pedantic -Wall -Werror -Wextra -g3 $(DEFS)


# ----- build + execution -----
.PHONY: all clean

all: ispalindrom

ispalindrom: ispalindrom.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ispalindrom.o: ispalindrom.c


# ----- clean up -----
clean:
	rm -rf *.o ispalindrom
