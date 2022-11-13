/**
 * @file forkFFT.h
 * @author Aiden Foster 11910604
 * @date 08.12.2021
 *
 * @brief Provides the ability to calculate the fourier transformation
**/

#ifndef FORKFFT_H
#define FORKFFT_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>
#include <float.h>

/** @brief default length of a malloc'ed array */
#define DEFAULT_ARRAY_LENGTH 16
/** @brief name of shared memory for quit flag */
#define SHM_QUIT_FLAG_NAME "/11910604_forkFFT_quitflag"
/** @brief semaphore to have mutual exclusion on quit flag (required for process counting)*/
#define SEM_WRITE_QUIT_NAME "/11910604_forkFFT_sem_wr_quit"
/** @brief number of children in which to fork */
#define NUM_CHILDREN 2
/** @brief seperator between tree branches */
#define TREE_SEPERATOR " "
/** @brief distance between tree branches */
#define TREE_SEPERATOR_DISTANCE 3

/**
 * @brief Circular buffer for use as shared memory
**/
struct sharedQuit {
    /**
     * @brief number of processes running
    **/
    unsigned int processes;
    /**
     * @brief Index at which to read next
    **/
    unsigned char quit;
};

/**
 * @brief An edge containing two verticies v1 and v2
**/
typedef struct sharedQuit sharedQuit;

#endif
