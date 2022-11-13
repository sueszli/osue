#include "circbuffer.h"
#include "graph.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

/**
 * @name circbuffer
 * @author Kurdo-Jaroslav Asinger, 01300351
 * @brief handling of sharedMem memory including semaphores and a circular buffer
 * @details create sharedMem memory with setup
 *          read from circular buffer with readBuf (supported by semaphores)
 *          write to circular buffer with writeBuf (supported by semaphores)
 *          cleanse sharedMem memory with cleanup
 * @date Nov/12/2021
 */

buffer *circBuffer;
int sharedMem;
sem_t *reader, *writer, *writelock;

/**
 * @brief creates sharedMem memory, circular buffer and semaphores
 * 
 * @param supervisor only truncate, initialize circBuffer and unlink semaphores if called by supervisor
 * @param progname program name is passed for error handling
 * @return int 0 on success, -1 on failure
 */
int setup(bool supervisor, char *progname)
{
    sharedMem = shm_open(MEM_NAME, O_RDWR | O_CREAT, 0600);
    if (sharedMem == -1)
    {
        fprintf(stderr, "ERROR at %s shm_open: %s\n", progname, strerror(errno));
        cleanup(supervisor, progname);
        return -1;
    }
    if (supervisor)
    {
        if (ftruncate(sharedMem, sizeof(buffer)) < 0)
        {
            fprintf(stderr, "ERROR at %s ftruncate: %s\n", progname, strerror(errno));
            cleanup(supervisor, progname);
            return -1;
        }
    }
    circBuffer = mmap(NULL, sizeof(*circBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, sharedMem, 0);
    if (circBuffer == MAP_FAILED)
    {
        fprintf(stderr, "ERROR at %s mmap: %s\n", progname, strerror(errno));
        cleanup(supervisor, progname);
        return -1;
    }
    // initialize the circular buffer
    if (supervisor)
    {
        circBuffer->readPos = 0;
        circBuffer->writePos = 0;
        circBuffer->state = 0;
    }
    // if semaphores were not unlinked due to previous errorous runs - therefore no error handling
    if (supervisor)
    {
        sem_unlink("01300351_read");
        sem_unlink("01300351_write");
        sem_unlink("01300351_lock");
        errno = 0;
    }
    // initialize with 0 - no reading allowed on a new buffer
    reader = sem_open("01300351_read", O_CREAT, 0600, 0);
    if (reader == SEM_FAILED)
    {
        fprintf(stderr, "ERROR at %s sem_open reader: %s\n", progname, strerror(errno));
        cleanup(supervisor, progname);
        return -1;
    }
    // initialize with BUFFER_SIZE - it is possible to write on every "empty" position
    writer = sem_open("01300351_write", O_CREAT, 0600, BUFFER_SIZE);
    if (writer == SEM_FAILED)
    {
        fprintf(stderr, "ERROR at %s sem_open writer: %s\n", progname, strerror(errno));
        cleanup(supervisor, progname);
        return -1;
    }
    // initialize with 1 - prevent write-after-write
    writelock = sem_open("01300351_lock", O_CREAT, 0600, 1);
    if (writelock == SEM_FAILED)
    {
        fprintf(stderr, "ERROR at %s sem_open writelock: %s\n", progname, strerror(errno));
        cleanup(supervisor, progname);
        return -1;
    }
    return 0;
}

/**
 * @brief used by supervisor to communicate to generator that it will terminate
 * 
 * @return true when state has not been changed yet
 * @return false when state has been changed (when stopSol has been called)
 */
bool isRunning(void)
{
    return circBuffer->state == 0;
}

/**
 * @brief changes the state of the circular buffer to signalize termination of supervisor
 * 
 */
void stopSol(void)
{
    circBuffer->state = 1;
}

/**
 * @brief reads the entry on the current read position and updates semaphores and read position
 *        if the buffer is not updated at current readPos, then the semaphoe reader will block until the
 *        update/write on this position was successful.
 * 
 * @param progname program name is passed for error handling
 * @return coloringSol the solution on the read position is returned if there were no issues with the semaphores
 *                     otherwise, an invalid solution is returned
 */
coloringSol readBuf(char *progname)
{
    coloringSol invalid;
    invalid.deletedCount = -1;
    if (sem_wait(reader) != 0)
    {
        if (errno == EINTR)
        {
            fprintf(stderr, "ERROR at %s sem_wait reader: %s\n", progname, strerror(errno));
            return invalid;
        }
        fprintf(stderr, "ERROR at %s sem_wait reader: %s\n", progname, strerror(errno));
        return invalid;
    }
    coloringSol newSol = circBuffer->solutions[circBuffer->readPos];
    if (sem_post(writer) != 0)
    {
        fprintf(stderr, "ERROR: at %s sem_post writer: %s\n", progname, strerror(errno));
        return invalid;
    }
    circBuffer->readPos = (circBuffer->readPos + 1) % BUFFER_SIZE;
    return newSol;
}

/**
 * @brief writes the given solution newSol to the current write position and updates semaphores and write position.
 *        if the entry on current writePos has not been read yet, then the semaphoe writer will block until the
 *        reading on this position was successful.
 * 
 * @param newSol a solution (found by generator)
 * @param progname program name is passed for error handling
 * @return int 0 on success, -1 on failure
 */
int writeBuf(coloringSol newSol, char *progname)
{
    if (sem_wait(writelock) != 0)
    {
        if (errno == EINTR)
        {
            fprintf(stderr, "ERROR at %s sem_wait writelock: %s\n", progname, strerror(errno));
            return -1;
        }
        fprintf(stderr, "ERROR at %s sem_wait writelock: %s\n", progname, strerror(errno));
        return -1;
    }
    if (sem_wait(writer) != 0)
    {
        if (errno == EINTR)
        {
            fprintf(stderr, "ERROR at %s sem_wait writer: %s\n", progname, strerror(errno));
            return -1;
        }
        fprintf(stderr, "ERROR at %s sem_wait writer: %s\n", progname, strerror(errno));
        return -1;
    }
    circBuffer->solutions[circBuffer->writePos] = newSol;
    if (sem_post(reader) != 0)
    {
        fprintf(stderr, "ERROR at %s sem_post reader: %s\n", progname, strerror(errno));
        return -1;
    }
    circBuffer->writePos = (circBuffer->writePos + 1) % BUFFER_SIZE;
    if (sem_post(writelock) != 0)
    {
        fprintf(stderr, "ERROR at %s sem_post writelock: %s\n", progname, strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * @brief cleans sharedMem memory (unmap buffer, deallocate/free allocated resources)
 * 
 * @param supervisor only unlink shared memory and semaphores if called by supervisor
 * @param progname program name is passed for error handling
 * @return int 0 on success, -1 on failure
 */
int cleanup(bool supervisor, char *progname)
{
    int succ = 0;
    if (munmap(circBuffer, sizeof(*circBuffer)) == -1)
    {
        fprintf(stderr, "ERROR at %s munmap: %s\n", progname, strerror(errno));
        succ = -1;
    }
    if (close(sharedMem) == -1)
    {
        fprintf(stderr, "ERROR at %s close shared memory: %s\n", progname, strerror(errno));
        succ = -1;
    }
    if (supervisor)
    {
        if (shm_unlink(MEM_NAME) == -1)
        {
            fprintf(stderr, "ERROR at %s shm_unlink: %s\n", progname, strerror(errno));
            succ = -1;
        }
    }
    if (sem_close(reader) == -1)
    {
        fprintf(stderr, "ERROR at %s sem_close reader: %s\n", progname, strerror(errno));
        succ = -1;
    }
    if (sem_close(writer) == -1)
    {
        fprintf(stderr, "ERROR at %s sem_close writer: %s\n", progname, strerror(errno));
        succ = -1;
    }
    if (sem_close(writelock) == -1)
    {
        fprintf(stderr, "ERROR at %s sem_close writelock: %s\n", progname, strerror(errno));
        succ = -1;
    }
    if (supervisor)
    {
        if (sem_unlink("01300351_read") == -1)
        {
            fprintf(stderr, "ERROR at %s sem_unlink reader: %s\n", progname, strerror(errno));
            succ = -1;
        }
        if (sem_unlink("01300351_write") == -1)
        {
            fprintf(stderr, "ERROR at %s sem_unlink writer: %s\n", progname, strerror(errno));
            succ = -1;
        }
        if (sem_unlink("01300351_lock") == -1)
        {
            fprintf(stderr, "ERROR at %s sem_unlink writelock: %s\n", progname, strerror(errno));
            succ = -1;
        }
    }
    return succ;
}
