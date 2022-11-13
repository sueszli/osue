CC = gcc
EXECUTABLE_NAME = myexpand
C_FLAGS = -std=c99 -Wall -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

.PHONY: clean all

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): *.o
	$(CC) $(C_FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(C_FLAGS) -g -c $^

documentation: *.c
	doxygen Doxyfile

run: 
	./$(EXECUTABLE_NAME)

clean:
	rm -f $(EXECUTABLE_NAME)
	rm -f *.o
	rm -rf html latex
