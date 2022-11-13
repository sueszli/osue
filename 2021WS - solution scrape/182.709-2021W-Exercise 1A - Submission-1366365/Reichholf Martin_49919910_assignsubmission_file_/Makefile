CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c 
LFLAGS =

PROGRAM = mygrep
MODULE1 = main
 
all: $(PROGRAM) 
 
$(PROGRAM): $(MODULE1).o 
	$(CC) $(LFLAGS) -o $(PROGRAM) $(MODULE1).o
 
$(MODULE1).o: $(MODULE1).c
	$(CC) $(CFLAGS) $(MODULE1).c
 

clean:
	rm -f $(MODULE1).o $(PROGRAM) 
