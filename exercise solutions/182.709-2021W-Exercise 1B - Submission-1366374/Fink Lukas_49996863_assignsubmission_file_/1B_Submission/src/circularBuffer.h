/**
 * @file circularBuffer.h
 * @author Lukas Fink 11911069
 * @date 10.11.2021
 *  
 * @brief circular buffer Module
 *
 * contains the circular buffer required for communication between generator and supervisor
 **/

#ifndef CB_H
#define CB_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>

#include "datatypes.h"

#define SHM_CIRCULAR_BUFFER "/11911069_myshm"
#define MAX_DATA (32) // 32 FeedbackArc's + 3 int equals max. 2316 Byte.

#define SEM_FREE "/11911069_sem_free"
#define SEM_USED "/11911069_sem_used"
#define SEM_WRITE "/11911069_sem_write"

/** Circular Buffer struct. 
 * @brief contains a buffer containing MAX_DATA FeedbackArcs
 */
struct CircularBuffer {
    struct FeedbackArc buffer[MAX_DATA];
    int readPos;
    int writePos;
    int terminate;
};

void readCB(struct FeedbackArc *dest, struct CircularBuffer *circularBuf, sem_t *semFree, sem_t *semUsed, char *programName);
void writeCB(struct FeedbackArc fbA, struct CircularBuffer *circularBuf, sem_t *semFree, sem_t *semUsed, char *programName);

#endif