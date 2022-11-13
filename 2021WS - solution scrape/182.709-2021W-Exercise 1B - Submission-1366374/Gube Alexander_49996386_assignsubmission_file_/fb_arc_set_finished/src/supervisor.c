/**
 * @file supervisor.c
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief fb_arc_set supervisor module
 *
 * This part is one of the two major modules for the feedback arc set project, which aims
 * to find good solutions for the feedback arc set problem. In order to communicate with the
 * other part (generators), this module is responsible for setting up, managing and cleaning up
 * a shared memory which is implemented as a circular buffer. A safe usage of the shm is established
 * by using three semaphores. Furthermore the supervisor reads from the circularBuffer and compares
 * solutions, which are provided by the generators and keeps the so-far best solution. As far as a new solution
 * is obtained, it is printed to stdout. In case there occurrs a solution of size 0 (graph is acyclic) the supervisor
 * terminates and notifies its generators to terminates as well.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/mman.h>                   //for shared memory
#include <fcntl.h>
//#include <semaphore.h>

#include "ErrorHandling.h"
#include "SemUtils.h"
#include "ShmUtils.h"

#define PROG_NAME "supervisor"

//flag to indicate end of program
volatile sig_atomic_t shouldExit = 0;


/**
 * General signal handling
 * @brief This function handles signals sent to the process
 * @param signal specifys the received signal
 * @details in case the supervisor receives a SIGINT or SIGTERM, it has to close, which is indicated by the global shouldExit flag
 */
static void handleSignals(int signal) {
    if(signal == SIGINT || signal == SIGTERM) {
        shouldExit = 1;
    }
}

/**
 * usage function.
 * @brief This function writes to stderr how the program is to be used
 */
static void usageMessage(void) {
    fprintf(stderr, "[%s] ERROR: program takes no arguments, Usage: %s\n", PROG_NAME, PROG_NAME);
    exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief This function is responsible for argument management, setting up the shm and its semaphores and observes the
 * circularBuffer for better solutions than the so-far best one.
 * @details As no arguments are intended, it is terminated as far as parameters are provided. Then the circularBuffer (truncated and mapped)
 * as well as the semaphores which manage the access to the shm are set up. In case there occurr errors, the program is terminated after releasing
 * all so-far initialized resoures. in a while loop, the supervisor moves the reading head of the circularBuffer and reads on the current position. The solution is
 * validated and in case it is better than the current best solution, the best solution is updated. In case a solution of size 0 is obtained or a respective signal occurres
 * the program is terminated. Before terminating, resources are cleaned up.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char** argv) {
    //supervisor takes no arguments
    while ((getopt(argc, argv, ":")) != -1 ){
        usageMessage();
    }
    //no pos arguments
    if((argc - optind) > 0) {
        usageMessage();
    }
    
    //reading head in circular buffer
    struct fbArc *currArc;
    //points to start of circular buffer
    struct fbArc *circularBuffer;
    //pointer to pos of write header
    int *writeHead;
    
    //best solution so far - restrict to 10
    int bestCount = 10;
    struct fbArc bestSolution;
    
    //signal handling
    struct sigaction signalAction;
    memset(&signalAction, 0, sizeof(signalAction));
    signalAction.sa_handler = handleSignals;
    sigaction(SIGINT, &signalAction, NULL);
    sigaction(SIGTERM, &signalAction, NULL);

    //init shared memory
    int fdBuffer;
    fdBuffer = shm_open(shmName, O_CREAT | O_RDWR, 0600);
    
    if(fdBuffer < 0) {
        failedWithError(PROG_NAME, "failed to create shared circular buffer", 1);
    }
    
    if(ftruncate(fdBuffer, bufferSize * sizeof(struct fbArc) + sizeof(int)) < 0) {
        closeSHM(fdBuffer, PROG_NAME);
        failedWithError(PROG_NAME, "failed to set size of circular buffer", 1);
    }
    
    //map shared memory to virtual file -> returns starting address of circular buffer
    circularBuffer = currArc = mmap(NULL, bufferSize * sizeof(struct fbArc) + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fdBuffer, 0);
    writeHead = (int*)(circularBuffer + bufferSize);
    //init to 0
    memset(circularBuffer, 0, bufferSize * sizeof(struct fbArc) + sizeof(int));
    
    if(circularBuffer == MAP_FAILED) {
        closeSHM(fdBuffer, PROG_NAME);
        unlinkSHM(shmName, PROG_NAME);
        failedWithError(PROG_NAME, "failed to map circular buffer to virtual file", 1);
    }
    
    //init semaphores
    sem_t *synchGem = sem_open(SYNCH_GEM, O_CREAT | O_EXCL, 0600, 1);
    if(synchGem == SEM_FAILED) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
        failedWithError(PROG_NAME, "failed to init synch semaphore", 1);
    }
    
    sem_t *readSem = sem_open(READ_SEM, O_CREAT | O_EXCL, 0600, 0);
    if(readSem == SEM_FAILED) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
        cleanupSEM(synchGem, SYNCH_GEM, PROG_NAME);
        failedWithError(PROG_NAME, "failed to init read semaphore", 1);
    }
    
    sem_t *writeSem = sem_open(WRITE_SEM, O_CREAT | O_EXCL, 0600, bufferSize);
    if(readSem == SEM_FAILED) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
        cleanupSEM(synchGem, SYNCH_GEM, PROG_NAME);
        cleanupSEM(readSem, READ_SEM, PROG_NAME);
        failedWithError(PROG_NAME, "failed to init write semaphore", 1);
    }

    //read head of circular buffer
    int pos = 0;
    //while loop which observes circular buffer and outputs so-far best results
    while((!shouldExit) && (bestCount > 0)) {
        if(sem_wait(readSem) < 0) {
            if(errno == EINTR) {
                continue;
            }
            else {
                cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
                cleanupSEM(synchGem, SYNCH_GEM, PROG_NAME);
                cleanupSEM(readSem, READ_SEM, PROG_NAME);
                cleanupSEM(writeSem, WRITE_SEM, PROG_NAME);
                failedWithError(PROG_NAME, "failed to wait on readSem", 1);
            }
        }
        //read solution and validate
        struct fbArc arc = *currArc;
        validate(arc, &bestSolution, &bestCount);
        pos++;
        currArc = circularBuffer + (pos % bufferSize);
        if(sem_post(writeSem) < 0) {
            cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
            cleanupSEM(synchGem, SYNCH_GEM, PROG_NAME);
            cleanupSEM(readSem, READ_SEM, PROG_NAME);
            cleanupSEM(writeSem, WRITE_SEM, PROG_NAME);
            failedWithError(PROG_NAME, "failed to post on writeSem", 1);
        }
    }
    
    //notify generators via write head to terminate
    if(sem_wait(writeSem) < 0) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
        cleanupSEM(synchGem, SYNCH_GEM, PROG_NAME);
        cleanupSEM(readSem, READ_SEM, PROG_NAME);
        cleanupSEM(writeSem, WRITE_SEM, PROG_NAME);
        failedWithError(PROG_NAME, "failed while notifying generators to quit.", 1);
    }
    if(sem_wait(synchGem) < 0) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
        cleanupSEM(synchGem, SYNCH_GEM, PROG_NAME);
        cleanupSEM(readSem, READ_SEM, PROG_NAME);
        cleanupSEM(writeSem, WRITE_SEM, PROG_NAME);
        failedWithError(PROG_NAME, "failed while notifying generators to quit.", 1);
    }
    *writeHead = -1;
    if(sem_post(synchGem) < 0) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
        cleanupSEM(synchGem, SYNCH_GEM, PROG_NAME);
        cleanupSEM(readSem, READ_SEM, PROG_NAME);
        cleanupSEM(writeSem, WRITE_SEM, PROG_NAME);
        failedWithError(PROG_NAME, "failed to post on notifying generators to quit.", 1);
    }
    if(sem_post(writeSem) < 0) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 1);
        cleanupSEM(synchGem, SYNCH_GEM, PROG_NAME);
        cleanupSEM(readSem, READ_SEM, PROG_NAME);
        cleanupSEM(writeSem, WRITE_SEM, PROG_NAME);
        failedWithError(PROG_NAME, "failed to post on notifying generators to quit.", 1);
    }
    
    int errorOnCleanup = 0;
    //cleanup shm
    if(unmap(circularBuffer, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(closeSHM(fdBuffer, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(unlinkSHM(shmName, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    //cleanup semaphores
    if(closeSEM(readSem, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(closeSEM(writeSem, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(closeSEM(synchGem, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(unlinkSEM(READ_SEM, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(unlinkSEM(WRITE_SEM, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(unlinkSEM(SYNCH_GEM, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(errorOnCleanup < 0) {
        failedWithError(PROG_NAME, "failed with error on cleanup", 1);
    }
    return EXIT_SUCCESS;
}

