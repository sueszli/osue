/** 
 * @file circularBuffer.h
 * @author Andreas Huber 11809629
 * @date 07.11.2021
 *
 * @brief Includes struct of the shared memory and declerations of functions in circularBuffer.c
 * 
 **/

#include <stdbool.h>


typedef struct sharedMemory{
    char buffer[1024];
    int rd_pos;
    int wr_pos;
    bool terminate;
} sharedMemory;

#define MIN(a,b) ((a) < (b) ? a : b) 
#define MAX(a,b) ((a) > (b) ? a : b) 

/**
 * supervisorOpenSemaphores function
 * @brief Open semaphores as a supervisor
 * @details Function to open the 3 semaphores free, used and wait. 
 *          If an error occures the semaphores will be closed before exiting 
 * @return 0 if opening of all semaphores was successfull
 * @return -1 if at least one opening of a semaphore was not successfull
**/
static int supervisorOpenSemaphores();

/**
 * generatorOpenSemaphores function
 * @brief Open semaphores as a generator
 * @details Function to open the 3 semaphores free, used and wait. 
 *          If an error occures the semaphores will be closed before exiting 
 * @return 0 if opening of all semaphores was successfull
 * @return -1 if at least one opening of a semaphore was not successfull
**/
static int generatorOpenSemaphores();

/**
 * supervisorCloseSemaphores function
 * @brief Closes semaphores as a supervisor
 * @details Function to close and unlink the 3 semaphores free, used and wait. 
 * @return 0 if closing and unlinking of all semaphores was successfull
 * @return -1 if at least one closing or unlinking of a semaphore was not successfull
**/
static int supervisorCloseSemaphores();

/**
 * generatorCloseSemaphores function
 * @brief Closes semaphores as a generator
 * @details Function to close the 3 semaphores free, used and wait. 
 * @return 0 if closing of all semaphores was successfull
 * @return -1 if at least one closing of a semaphore was not successfull
**/
static int generatorCloseSemaphores();

/**
 * supervisorOpenSharedMemory function
 * @brief opens shared memory as a supervisor
 * @details function to open a hared memory struct and return it 
 * @return NULL if opening of the shared memory or mmap failed
 * @return sharedMemory pointer if sharedMemory was successfully openend and mapped
**/
static sharedMemory* supervisorOpenSharedMemory();

/**
 * generatorOpenSharedMemory function
 * @brief opens shared memory as a generator
 * @details function to open a shared memory struct and return it 
 * @return NULL if opening of the shared memory or mmap failed
 * @return sharedMemory pointer if sharedMemory was successfully openend and mapped
**/
static sharedMemory* generatorOpenSharedMemory();

/**
 * supervisorCloseSharedMemory function
 * @brief closes shared memory as a supervisor
 * @details function to unmap and close and unlink a shared memory 
 * @param shmem sharedMemory that should be closed
 * @return 0 if munmap, close and unlink were successfull
 * @return -1 if munmap, close or unlink were not successfull
**/
static int supervisorCloseSharedMemory(sharedMemory* shmem);

/**
 * generatorCloseSharedMemory function
 * @brief closes shared memory as a generator
 * @details function to unmap and close a shared memory 
 * @param shmem sharedMemory that should be closed
 * @return 0 if munmap and close were successfull
 * @return -1 if munmap or close was not successfull
**/
static int generatorCloseSharedMemory(sharedMemory* shmem);

/**
 * supervisorOpenCircularBuffer function
 * @brief opens a circular buffer as a supervisor
 * @details function to open a shared memory, semaphores and return a sharedMemory that will function as a circularBuffer
 * initializes the circular buffer with rd-wr position 0.
 * @return NULL if opening of semaphores or shared memory failed
 * @return circular buffer (as shared memory struct) if opening of semaphores and shared memory were successful. 
**/
sharedMemory* supervisorOpenCircularBuffer();

/**
 * generatorOpenCircularBuffer function
 * @brief opens a circular buffer as a generator
 * @details function to open a shared memory, semaphores and return a sharedMemory that will function as a circularBuffer
 * @return NULL if opening of semaphores or shared memory failed
 * @return circular buffer (as shared memory struct) if opening of semaphores and shared memory were successful. 
**/
sharedMemory* generatorOpenCircularBuffer();

/**
 * readCircularBuffer function
 * @brief read data from the circular buffer 
 * @details function for the generator to read from the circular buffer
 * @param shmem circular buffer to which data will be written 
 * @return NULL if sem_wait(semUsed) returns -1 (blocking)
 * @return char* to data-entry read from circular buffer
**/
char* readCircularBuffer(sharedMemory* shmem);

/**
 * writeCircularBuffer function
 * @brief write data to a circular buffer
 * @details function for a sever to write to a circular buffer 
 * @param shmem circular buffer to which data will be written
 * @return -1 if semWait or semFree blocks or the circular buffer is terminated
 * @return 0 if writing is successful.
**/
int writeCircularBuffer(sharedMemory* shmem, char* data);

/**
 * supervisorCloseCircularBuffer function
 * @brief closes a circular buffer as a supervisor
 * @details function to close shared memory and semaphores used by the circular buffer
 * @return -1 if closing the shared memory or semaphores failed
 * @return 0 if closing the shared memory and semaphores were successful. 
**/
int supervisorCloseCircularBuffer(sharedMemory* shmem);

/**
 * generatorCloseCircularBuffer function
 * @brief closes a circular buffer as a generator
 * @details function to close shared memory and semaphores used by the circular buffer
 * @return -1 if closing the shared memory or semaphores failed
 * @return 0 if closing the shared memory and semaphores were successful. 
**/
int generatorCloseCircularBuffer(sharedMemory* shmem);