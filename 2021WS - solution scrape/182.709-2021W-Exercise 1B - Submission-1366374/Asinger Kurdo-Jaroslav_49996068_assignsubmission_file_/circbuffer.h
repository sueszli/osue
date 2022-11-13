#ifndef CIRCBUFFER_H

#define CIRCBUFFER_H
#include <stdbool.h>
#include "graph.h"

#define BUFFER_SIZE 40
#define MEM_NAME "01300351"

/* the circular buffer structconsists of read position, write position,
the state (if supervisor is still running) and the buffer itself, realized as a coloringSol array*/
typedef struct {
    unsigned int readPos;
    unsigned int writePos;
    unsigned int state;
    coloringSol solutions[BUFFER_SIZE];
} buffer;

int setup(bool supervisor, char *progname);
int cleanup(bool supervisor, char *progname);

void stopSol(void);

bool isRunning(void);
int writeBuf(coloringSol newSol, char *progname);
coloringSol readBuf(char *progname);

#endif