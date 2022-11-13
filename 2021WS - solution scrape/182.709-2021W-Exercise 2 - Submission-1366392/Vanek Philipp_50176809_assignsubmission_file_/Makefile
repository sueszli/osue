# forkFFT from Philipp Vanek 12022484
CC = gcc
C_FLAGS = -std=c99 -Wall -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LINK_LIBS = -lrt -lpthread -lm
EXECUTABLE_NAME = forkFFT

.PHONY: clean all

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): forkFFT.o
	$(CC) $(C_FLAGS) $(LINK_LIBS) -o $@ $^



%.o: %.c
	$(CC) $(C_FLAGS) -c $^

documentation: *.c
	doxygen Doxygen


clean: 
	rm -f $(EXECUTABLE_NAME)
	rm -f *.o
	rm -rf html latex
