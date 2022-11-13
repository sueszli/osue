#include "CircularBuffer.h"

#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

/**
 * Datastructure allocated in the shared memory.
 * 
 * closeRequest is true when a close was requested
 * readPos contains the current reading position
 * writePos the current writing position
 * buffer points at the memory of size bufferSize used for communication
 */
struct SharedMemoryContent
{
    bool closeRequested;
    uint32_t readPos;
    uint32_t writePos;
    char buffer[];
};

CircularBufferErrors 
CircularBufferCreate(CircularBuffer* circularBuffer, const char* name, size_t bufferSize, int oflags)
{
    CircularBufferErrors error;
    assert(circularBuffer);
    circularBuffer->myMemorySize = bufferSize;
    size_t sharedMemorySize = sizeof(struct SharedMemoryContent) + bufferSize;
    circularBuffer->mySharedMemoryFD = shm_open(name, oflags, 0777);
    if (circularBuffer->mySharedMemoryFD < 0)
    {
        return CBSharedMemoryError;
    }
    if (ftruncate(circularBuffer->mySharedMemoryFD, sharedMemorySize) < 0)
    {
        error = CBSharedMemoryError;
        goto error_ftruncate;
    }
    circularBuffer->mySharedMemory = mmap(NULL, sharedMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, 
                                            circularBuffer->mySharedMemoryFD, 0);
    if (circularBuffer->mySharedMemory == MAP_FAILED)
    {
        error = CBSharedMemoryError;
        goto error_mmap;
    }
    char nameBuffer[NAME_MAX-4+1];
    // Only god can help us if name is not null terminated
    strncpy(nameBuffer, name, sizeof(nameBuffer));
    size_t len = strlen(nameBuffer);
    if (len + 1 >= sizeof(nameBuffer))
    {
        error = CBSemaphoreError;
        goto error_nameBuffer;
    }
    
    nameBuffer[len] = 'u';
    circularBuffer->myUsedSem = sem_open(nameBuffer, oflags, 0777, 0);
    if (circularBuffer->myUsedSem == SEM_FAILED)
    {
        error = CBSemaphoreError;
        goto error_usedSem;
    }
    
    nameBuffer[len] = 'f';
    circularBuffer->myFreeSem = sem_open(nameBuffer, oflags, 0777, bufferSize);
    if (circularBuffer->myFreeSem == SEM_FAILED)
    {
        error = CBSemaphoreError;
        goto error_freeSem;
    }
    
    nameBuffer[len] = 'x';
    circularBuffer->myExclusiveWriteSem = sem_open(nameBuffer, oflags, 0777, 1);
    if (circularBuffer->myExclusiveWriteSem == SEM_FAILED)
    {
        error = CBSemaphoreError;
        goto error_exclWriteSem;
    }
    
    nameBuffer[len] = 'c';
    circularBuffer->myExclusiveCloseRequestSem = sem_open(nameBuffer, oflags, 0777, 1);
    if (circularBuffer->myExclusiveCloseRequestSem == SEM_FAILED)
    {
        error = CBSemaphoreError;
        goto error_exclRequestSem;
    }
    
    circularBuffer->myUsedOFlags = oflags;
    // allocate the name + one character for the semaphore suffixes
    // + a null terminator
    circularBuffer->myUsedName = malloc(len + 2);
    if (!circularBuffer->myUsedName)
    {
        error = CBMemoryAllocationError;
        goto error_memoryAlloc;
    }
    strncpy(circularBuffer->myUsedName, name, len + 2);
    circularBuffer->mySharedMemory->closeRequested = false;
    circularBuffer->mySharedMemory->readPos = 0;
    circularBuffer->mySharedMemory->writePos = 0;
    return CBSuccess;        
    
error_memoryAlloc:
    (void)sem_close(circularBuffer->myExclusiveCloseRequestSem);
    if ((oflags & O_CREAT) && (oflags & O_EXCL))
    {
        nameBuffer[len] = 'c';
        (void)sem_unlink(nameBuffer);
    }
    
error_exclRequestSem:
    (void)sem_close(circularBuffer->myExclusiveWriteSem);
    if ((oflags & O_CREAT) && (oflags & O_EXCL))
    {
        nameBuffer[len] = 'x';
        (void)sem_unlink(nameBuffer);
    }
error_exclWriteSem:
    (void)sem_close(circularBuffer->myFreeSem);
    if ((oflags & O_CREAT) && (oflags & O_EXCL))
    {
        nameBuffer[len] = 'f';
        (void)sem_unlink(nameBuffer);
    }
error_freeSem:
    (void)sem_close(circularBuffer->myUsedSem);
    if ((oflags & O_CREAT) && (oflags & O_EXCL))
    {
        nameBuffer[len] = 'u';
        (void)sem_unlink(nameBuffer);
    }
error_usedSem:
error_nameBuffer:
error_mmap:
error_ftruncate:
    close(circularBuffer->mySharedMemoryFD);
    shm_unlink(name);
    return error;
}

void CircularBufferDestroy(CircularBuffer* circularBuffer)
{
    (void)munmap(circularBuffer->mySharedMemory, sizeof(struct SharedMemoryContent) + circularBuffer->myMemorySize);
    (void)close(circularBuffer->mySharedMemoryFD);
    int oflags = circularBuffer->myUsedOFlags;
    if ((oflags & O_CREAT) && (oflags & O_EXCL))
    {
        (void)shm_unlink(circularBuffer->myUsedName);
    }
    (void)sem_close(circularBuffer->myUsedSem);
    (void)sem_close(circularBuffer->myFreeSem);
    (void)sem_close(circularBuffer->myExclusiveWriteSem);
    (void)sem_close(circularBuffer->myExclusiveCloseRequestSem);
    if ((oflags & O_CREAT) && (oflags & O_EXCL))
    {
        size_t lastChar = strlen(circularBuffer->myUsedName);
        circularBuffer->myUsedName[lastChar] = 'u';
        (void)sem_unlink(circularBuffer->myUsedName);
        circularBuffer->myUsedName[lastChar] = 'f';
        (void)sem_unlink(circularBuffer->myUsedName);
        circularBuffer->myUsedName[lastChar] = 'x';
        (void)sem_unlink(circularBuffer->myUsedName);
        circularBuffer->myUsedName[lastChar] = 'c';
        (void)sem_unlink(circularBuffer->myUsedName);
        circularBuffer->myUsedName[lastChar] = '\0';
    }
    free(circularBuffer->myUsedName);
}

void CircularBufferRead(CircularBuffer* circularBuffer, void* ptr, size_t size)
{
    char* dest = ptr;
    for (size_t i = 0; i < size; i++)
    {
        sem_wait(circularBuffer->myUsedSem);
        *dest = circularBuffer->mySharedMemory->buffer[circularBuffer->mySharedMemory->readPos];
        dest++;
        sem_post(circularBuffer->myFreeSem);
        circularBuffer->mySharedMemory->readPos += 1;
        circularBuffer->mySharedMemory->readPos %= circularBuffer->myMemorySize;
    }
}

void CircularBufferWrite(CircularBuffer* circularBuffer, const void* ptr, size_t size)
{
    const char* src = ptr;
    sem_wait(circularBuffer->myExclusiveWriteSem);
    // Don't write if a close was requested. Noone will hear our scream
    // and we may otherwise deadlock waiting for someone to read our
    // message forever :(
    for (size_t i = 0; i < size && !CircularBufferCloseRequested(circularBuffer); i++)
    {
        sem_wait(circularBuffer->myFreeSem);
        circularBuffer->mySharedMemory->buffer[circularBuffer->mySharedMemory->writePos] = *src;
        src++;
        sem_post(circularBuffer->myUsedSem);
        circularBuffer->mySharedMemory->writePos += 1;
        circularBuffer->mySharedMemory->writePos %= circularBuffer->myMemorySize;
    }
    sem_post(circularBuffer->myExclusiveWriteSem);
}

bool CircularBufferCloseRequested(CircularBuffer* circularBuffer)
{
    sem_wait(circularBuffer->myExclusiveCloseRequestSem);
    bool result = circularBuffer->mySharedMemory->closeRequested;
    sem_post(circularBuffer->myExclusiveCloseRequestSem);
    return result;
}

void CircularBufferRequestClose(CircularBuffer* circularBuffer)
{
    sem_wait(circularBuffer->myExclusiveCloseRequestSem);
    circularBuffer->mySharedMemory->closeRequested = true;
    sem_post(circularBuffer->myExclusiveCloseRequestSem);
    // We may have managed to request the close here but generators are
    // almost CERTAINLY stuck trying to write to the circular buffer
    // when noone is reading. We will "read" via sem_post until they
    // are all done writing and exit due to the close request
    int value;
    do
    {
        if (sem_getvalue(circularBuffer->myExclusiveWriteSem, &value))
        {
            // can't really handle the error but we should at least
            // break out of the loop then
            break;
        }
        sem_post(circularBuffer->myFreeSem);
    }
    while (value <= 0);
}
