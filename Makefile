.PHONY: all clean


# compiler config 
COMPILER = gcc-12
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
LOG_MODE = -DDEBUG
DEFAULT_FLAGS = $(LOG_MODE) -std=c99 -pedantic -Wall -g $(DEFS)
VERBOSE_FLAGS = $(LOG_MODE) -std=c99 -pedantic -Wall -Werror -Wextra -g3 $(DEFS)


# build
all: ispalindrom

ispalindrom: ispalindrom.o
	$(COMPILER) -o $@ $^

%.o: %.c
	$(COMPILER) $(DEFAULT_FLAGS) -c -o $@ $<

ispalindrom.o: ispalindrom.c


# clean up
clean:
	rm -rf *.o ispalindrom
