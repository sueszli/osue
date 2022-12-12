DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
override CFLAGS += -Wall -Wextra -Wwrite-strings -Wconversion -g3 -std=c99 -pedantic $(DEFS)
# --> added -Wall -Wextra -Wwrite-strings -Wconversion -g3
override LDFLAGS +=
override LIBS +=

TARGET = client
OBJS = client.o

LDOBJS = client_lib.o

# rules

.PHONY : all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	gcc $(LDFLAGS) -o $@ $^ $(LDOBJS) $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

# dependencies

client.o: client.c client.h
