DEFS = -D DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
override CFLAGS += -pedantic -Wall -std=c99 -g $(DEFS)
TARGET = mygrep
OBJS = mygrep.o
.PHONY : all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	gcc $(CFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<
    
clean:
	rm -rf $(TARGET) $(OBJS)
