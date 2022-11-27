# @author Fabian Hagmann (12021352)
# @brief Program names (supervisor, generator)

CC = gcc

C_FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
L_FLAGS = -lrt -pthread

SOURCES1 = supervisor.c
SOURCES2 = generator.c
OBJECTS1 = $(SOURCES1:.c=.o)
OBJECTS2 = $(SOURCES2:.c=.o)
EXECUTABLE_NAME1 = supervisor
EXECUTABLE_NAME2 = generator

.PHONY: clean all

all: $(EXECUTABLE_NAME1) $(EXECUTABLE_NAME2)

$(EXECUTABLE_NAME1): $(OBJECTS1)
	$(CC) -o $@ $^ $(L_FLAGS)

$(EXECUTABLE_NAME2): $(OBJECTS2)
	$(CC) -o $@ $^ $(L_FLAGS)

	rm -f $(OBJECTS1) $(OBJECTS2)

%.o: %.c
	$(CC) $(C_FLAGS) -c $^

documentation:
	doxygen Doxyfile

clean:
	rm -f $(EXECUTABLE_NAME1) $(EXECUTABLE_NAME2)
	rm -f $(OBJECTS1) $(OBJECTS2)
	rm -f *.o
	rm -rf html latex