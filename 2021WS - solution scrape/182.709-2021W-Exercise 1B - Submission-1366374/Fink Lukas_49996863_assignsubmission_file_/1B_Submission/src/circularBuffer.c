/**
 * @file circularBuffer.c
 * @author Lukas Fink 11911069
 * @date 10.11.2021
 *  
 * @brief circular buffer Module implementation
 *
 * contains functions for reading from and writing to the circular buffer
 **/

#include "circularBuffer.h"
static int deepCopyArc(struct FeedbackArc *dest, struct FeedbackArc src);

/**
 * read function
 * @brief read from the circular buffer
 * @details deep copy of the buffer to the given destination
 * @param dest destination for the data to be copied
 * @param circularBuf the circular Buffer where data is read from
 * @param semFree semaphore which value equals the free space of the buffer
 * @param semUsed semaphore which value equals the used space of the buffer
 * @param programName name of the program calling the function
 **/
void readCB(struct FeedbackArc *dest, struct CircularBuffer *circularBuf, sem_t *semFree, sem_t *semUsed, char *programName) {
    if (sem_wait(semUsed) == -1) {
        if (errno != EINTR) {
            fprintf(stderr, "%s: sem_wait: failed!\n", programName);
            exit(EXIT_FAILURE);
        }
    }

    int readPos = circularBuf->readPos;
    int sucDCA = deepCopyArc(dest, circularBuf->buffer[readPos]);
    if (sucDCA == -1) {
        fprintf(stderr, "%s: deepCopyArc: failed!\n", programName);
        exit(EXIT_FAILURE);
    }

    readPos++;
    readPos %= MAX_DATA;
    circularBuf->readPos = readPos;

    if (sem_post(semFree) == -1) {
        if (errno != EINTR) {
            fprintf(stderr, "%s: sem_post: failed!\n", programName);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * write function
 * @brief write to the circular buffer
 * @details write the given Feedback-Arc into the circular Buffer
 * @param fba Feedback-Arc to be written into the buffer
 * @param circularBuf the circular Buffer where data is written to
 * @param semFree semaphore which value equals the free space of the buffer
 * @param semUsed semaphore which value equals the used space of the buffer
 * @param programName name of the program calling the function
 **/
void writeCB(struct FeedbackArc fbA, struct CircularBuffer *circularBuf, sem_t *semFree, sem_t *semUsed, char *programName) {
    if (sem_wait(semFree) == -1) {
        if (errno != EINTR) {
            fprintf(stderr, "%s: sem_wait: failed!\n", programName);
            exit(EXIT_FAILURE);
        }
    }
    
    int writePos = circularBuf->writePos;
    circularBuf->buffer[writePos] = fbA;

    writePos++;
    writePos %= MAX_DATA;
    circularBuf->writePos = writePos;

    if (sem_post(semUsed) == -1) {
        if (errno != EINTR) {
            fprintf(stderr, "%s: sem_post: failed!\n", programName);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * deep copy function
 * @brief deep copy of an Feedback Arc
 * @details copys values of src to dest and terminates
 * the edge array with an edge with negative values.
 * @param dest destination for the data to be copied
 * @param src source of data to be copied from
 * @return 0 on success, -1 on failure
 **/
static int deepCopyArc(struct FeedbackArc *dest, struct FeedbackArc src) {
    if (dest == NULL) {
        return -1;
    }
    int n = 0;
    while (n <= MAX_EDGES) {
        dest->feedback[n].start = src.feedback[n].start;
        dest->feedback[n].end = src.feedback[n].end;
        n++;
    }
    dest->feedback[MAX_EDGES].start = -1;
    dest->feedback[MAX_EDGES].end = -1;
    return 0;
}