# @file Makefile
# @author Yahya Jabary <mailto: yahya.jabary@tuwien.ac.at>
# @date 09.11.2022
# 
# @brief Makefile with multiple compile options to choose from.
# 			 Also see the ./run.sh script for running this project with test data.


.PHONY: all clean


COMPILER = gcc-12
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
SHOW_LOG = -DDEBUG
# select a compile mode
DEFAULT_FLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
DEFAULT_LOG_FLAGS = $(SHOW_LOG) -std=c99 -pedantic -Wall -g $(DEFS)
VERBOSE_FLAGS = $(SHOW_LOG) -std=c99 -pedantic -Wall -Werror -Wextra -g3 $(DEFS)


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
