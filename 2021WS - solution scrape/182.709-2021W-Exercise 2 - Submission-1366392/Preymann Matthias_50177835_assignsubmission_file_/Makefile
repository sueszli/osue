# cpair by Matthias Preymann (12020638)
CC = gcc
CFLAGS =  -std=c99 -Wall -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
EXECUTABLE_NAME= cpair
LLIBS= -lm

.PHONY: clean all

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): *.o
	$(CC) $(CFLAGS) $(LLIBS)  -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS)  -c $^

documentation:
	doxygen Doxygen

run:
	./$(EXECUTABLE_NAME)

clean:
	rm $(EXECUTABLE_NAME)
	rm -f *.o
	rm -rf html latex
