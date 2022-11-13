#ifndef CIRCULAR_BUFFER_HEADER
#define CIRCULAR_BUFFER_HEADER

/**
 * CircularBuffer.h & CircularBuffer.c
 * 
 * @author Markus Böck, 12020632
 * @date 2021.11.10
 * @brief Circular buffer datastructure responsible for IPC
 */

#include <stddef.h>
#include <semaphore.h>
#include <stdbool.h>

struct SharedMemoryContent;

/**
 * @brief Datatype representing a CircularBuffer. 
 * 
 * Fields of the struct are not meant to be accessed by users. 
 * Instead use the functions listed below. 
 * 
 * Typical usage of the struct is via simply making a variable with it
 * and then initializing it via CircularBufferCreate. 
 * Makes sure to use CircularBufferDestroy when done using it.
 * 
 */
typedef struct CircularBuffer
{
    sem_t* myUsedSem;
    sem_t* myFreeSem;
    sem_t* myExclusiveWriteSem;
    sem_t* myExclusiveCloseRequestSem;
    struct SharedMemoryContent* mySharedMemory;
    size_t myMemorySize;
    int mySharedMemoryFD;
    int myUsedOFlags;
    char* myUsedName;
} CircularBuffer;

/**
 * @brief Errors returned by the CircularBufferCreate function
 */
typedef enum CircularBufferErrors
{
    CBSuccess,
    CBSharedMemoryError,
    CBSemaphoreError,
    CBMemoryAllocationError,
} CircularBufferErrors;

/**
 * @brief Initializes a CircularBuffer
 * @param circularBuffer Pointer to the circularBuffer to initialze.
 * @param name Name to use as a key for multiple processes to connect with
 * @param bufferSize Size the buffer should have in bytes. The actual shared memory might be larger
 * @param oflags Flags used to create the shared memory and semaphores. Same as open
 * @return CircularBufferErrors enum
 */
CircularBufferErrors 
CircularBufferCreate(CircularBuffer* circularBuffer, const char* name, size_t bufferSize, int oflags);

/**
 * @brief Destroys the CircularBuffer
 * @param circularBuffer Pointer to the circularBuffer to destroy
 * 
 * Frees all resources used in the implementation. If the oflags that 
 * were used to create the CircularBuffer contained both O_CREAT and
 * O_EXCL then this method will also unlink any shared memory and 
 * semaphores created
 */
void CircularBufferDestroy(CircularBuffer* circularBuffer);

/**
 * @brief read data from the CircularBuffer
 * @param circularBuffer Pointer to the CircularBuffer to read from
 * @param ptr Destination to write to
 * @param size Amount of bytes to read from the buffer, into ptr
 * 
 * This function reads size bytes from the circular buffer and stores 
 * them into ptr. If ptr does not point to memory that is large enough
 * it is UB. 
 * 
 * Concurrent readers are not supported and will result in UB. Reading
 * while other processes are writing to the circular buffer is well
 * defined. 
 * 
 * This call is potentially blocking if not enough bytes have been
 * written to the CircularBuffer yet. It will resume as soon as another
 * processes writes enough data into the buffer.
 */
void CircularBufferRead(CircularBuffer* circularBuffer, void* ptr, size_t size);

/**
 * @brief write data to the CircularBuffer
 * @param circularBuffer Pointer to the CircularBuffer to write to
 * @param ptr Source bytes to read from
 * @param size Amount of bytes to read from ptr into the buffer
 * 
 * This function writes size bytes from ptr into the circular buffer.
 * If ptr does not point to memory that is large enough it is UB.
 * 
 * Concurrent writers are supported via exclusive access. One may also
 * have at most one reader reading from the buffer while writing.
 * 
 * This call is potentially blocking. If the buffer is full it'll
 * block until enough bytes have been read from the buffer to write
 * the payload. It may also block if another process is currently writing
 * to the buffer.
 * 
 * Transmission is done bytes by bytes. The bufferSize used in 
 * CircularBufferCreate is simply there fore optimizations but does
 * not affect correctness. You may transmit objects larger than the 
 * bufferSize. This function may in such case simply block if there is
 * no other process reading the data from the buffer
 */
void CircularBufferWrite(CircularBuffer* circularBuffer,const void* ptr, size_t size);

/**
 * @brief Checks whether the closing of the buffer has been requested
 * @param circularBuffer Pointer to the CircularBuffer
 * @return True if closing the buffer has been requested, false otherwise
 * 
 * This method is used to check whether a reader has requested to close
 * the circular buffer. If this method returns true, reading from the 
 * circular buffer is not allowed anymore. Writing to the buffer
 * afterwards becomes a NOOP
 */
bool CircularBufferCloseRequested(CircularBuffer* circularBuffer);

/**
 * @brief Requests all users of the buffer to stop using it
 * @param circularBuffer Pointer to the CircularBuffer
 * 
 * This method requests all users of the CircularBuffer to stop using
 * it and destroy it. This function may be called concurrently to
 * CircularBufferCloseRequested and CircularBufferWrite. If this 
 * function is called while a process is reading from the buffer it is
 * UB. Any writers that are currently within CircularBufferWrite while
 * this method is called will be unblocked and may not finish writing.
 * Any subsequent write attempts will result in a NOOP
 */
void CircularBufferRequestClose(CircularBuffer* circularBuffer);

#endif
