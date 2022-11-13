#program: 	ispalindrom.c
#author:	Lukas Crnkoci (01529217)
CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
TARGET = ispalindrom

all:    $(TARGET)

$(TARGET): $(TARGET).c
		$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

%.o: %.c
	$(CC) $(CFLAGS) -c # -c should not be set in CFLAGS, because it will otherwise deny write permissions  
clean: 
	$(RM) $(TARGET)