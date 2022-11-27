# mygrep from Philipp Vanek 12022484
CC = gcc
C_FLAGS = -std=c99 -Wall -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
EXECUTABLE_NAME = mygrep

.PHONY: clean all

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): *.o
	$(CC) $(C_FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(C_FLAGS) -c $^

documentation: *.c
	doxygen Doxygen

run: 
	./$(EXECUTABLE_NAME)


clean: 
	rm $(EXECUTABLE_NAME)
	rm -f *.o
	rm -rf html latex
