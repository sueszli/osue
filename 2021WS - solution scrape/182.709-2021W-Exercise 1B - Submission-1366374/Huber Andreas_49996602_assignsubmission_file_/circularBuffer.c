/** 
 * @file circularBuffer.c
 * @author Andreas Huber 11809629
 * @date 07.11.2021
 *
 * @brief Includes all necessary things for semaphores, shared memory and the circular buffer as well as 
 *as well as the function to interact with these things.
 * 
 * @details 
 **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

//Semaphore includes
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "circularBuffer.h"
#include <unistd.h>


//Constants for semaphore- and shhared-memory names as required by assignement
#define SEM_FREE "11809629-free"
#define SEM_USED "11809629-used"
#define SEM_WAIT "11809629-wait"
#define SHMEM "11809629-sharedMemory"

//Global variables used by all methods

sem_t* semFree;
sem_t* semUsed;
sem_t* semWait;

int shmfd;

/**
 * supervisorOpenSemaphores function
 * @brief Open semaphores as a supervisor
 * @details Function to open the 3 semaphores free, used and wait. 
 *          If an error occures the semaphores will be closed before exiting 
 * @return 0 if opening of all semaphores was successfull
 * @return -1 if at least one opening of a semaphore was not successfull
**/
static int supervisorOpenSemaphores(){
    if((semFree = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, 1024))==SEM_FAILED){
        return -1;
    }
    if((semUsed = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0))==SEM_FAILED){
        sem_close(semFree);
        sem_unlink(SEM_FREE);
        return -1;
    }
    if((semWait = sem_open(SEM_WAIT, O_CREAT | O_EXCL, 0600, 1))==SEM_FAILED){   
        sem_close(semFree);
        sem_close(semUsed);
        sem_unlink(SEM_FREE);
        sem_unlink(SEM_USED);
        return -1;
    }
    return 0;
}

/**
 * generatorOpenSemaphores function
 * @brief Open semaphores as a generator
 * @details Function to open the 3 semaphores free, used and wait. 
 *          If an error occures the semaphores will be closed before exiting 
 * @return 0 if opening of all semaphores was successfull
 * @return -1 if at least one opening of a semaphore was not successfull
**/
static int generatorOpenSemaphores(){
    if((semFree = sem_open(SEM_FREE, 0))==SEM_FAILED){
        return -1;
    }
    if((semUsed = sem_open(SEM_USED, 0))==SEM_FAILED){
        sem_close(semFree);
        return -1;
    }
    if((semFree = sem_open(SEM_WAIT, 0))==SEM_FAILED){
        sem_close(semFree);
        sem_close(semUsed);
        return -1;
    }
    return 0;
}

/**
 * supervisorCloseSemaphores function
 * @brief Closes semaphores as a supervisor
 * @details Function to close and unlink the 3 semaphores free, used and wait. 
 * @return 0 if closing and unlinking of all semaphores was successfull
 * @return -1 if at least one closing or unlinking of a semaphore was not successfull
**/
static int supervisorCloseSemaphores(){
    int returnValue = 0;    
    returnValue = MIN(returnValue, sem_close(semFree));
    returnValue = MIN(returnValue, sem_close(semUsed));
    returnValue = MIN(returnValue, sem_close(semWait));
    returnValue = MIN(returnValue, sem_unlink(SEM_FREE));
    returnValue = MIN(returnValue, sem_unlink(SEM_USED));
    returnValue = MIN(returnValue, sem_unlink(SEM_WAIT));
    return returnValue;
}

/**
 * generatorCloseSemaphores function
 * @brief Closes semaphores as a generator
 * @details Function to close the 3 semaphores free, used and wait. 
 * @return 0 if closing of all semaphores was successfull
 * @return -1 if at least one closing of a semaphore was not successfull
**/
static int generatorCloseSemaphores(){
    int returnValue = 0;    
    returnValue = MIN(returnValue, sem_close(semFree));
    returnValue = MIN(returnValue, sem_close(semUsed));
    returnValue = MIN(returnValue, sem_close(semWait));
    return returnValue;
}

/**
 * supervisorOpenSharedMemory function
 * @brief opens shared memory as a supervisor
 * @details function to open a hared memory struct and return it 
 * @return NULL if opening of the shared memory or mmap failed
 * @return sharedMemory pointer if sharedMemory was successfully openend and mapped
**/
static sharedMemory* supervisorOpenSharedMemory(){
    if((shmfd = shm_open(SHMEM, O_RDWR | O_CREAT | O_EXCL, 0600))==-1){
        return NULL;
    }
    if(ftruncate(shmfd, sizeof(sharedMemory))<0){
        close(shmfd);
        shm_unlink(SHMEM);
        return NULL;
    }
    sharedMemory* shmem;
    if((shmem = mmap(NULL, sizeof(*shmem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0))==MAP_FAILED){
        close(shmfd);
        shm_unlink(SHMEM);
        return NULL;
    }
    return shmem;
}

/**
 * generatorOpenSharedMemory function
 * @brief opens shared memory as a generator
 * @details function to open a shared memory struct and return it 
 * @return NULL if opening of the shared memory or mmap failed
 * @return sharedMemory pointer if sharedMemory was successfully openend and mapped
**/
static sharedMemory* generatorOpenSharedMemory(){
    if((shmfd = shm_open(SHMEM, O_RDWR, 0600))==-1){
        return NULL;
    }
    sharedMemory* shmem;
    if((shmem = mmap(NULL, sizeof(*shmem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0))==MAP_FAILED){
        close(shmfd);
        return NULL;
    }
    return shmem;
}

/**
 * supervisorCloseSharedMemory function
 * @brief closes shared memory as a supervisor
 * @details function to unmap and close and unlink a shared memory 
 * @param shmem sharedMemory that should be closed
 * @return 0 if munmap, close and unlink were successfull
 * @return -1 if munmap, close or unlink were not successfull
**/
static int supervisorCloseSharedMemory(sharedMemory* shmem){
    int returnValue = 0;    
    returnValue = MIN(returnValue, munmap(shmem, sizeof(*shmem)));
    returnValue = MIN(returnValue, close(shmfd));
    returnValue = MIN(returnValue, shm_unlink(SHMEM));
    return returnValue;
}

/**
 * generatorCloseSharedMemory function
 * @brief closes shared memory as a generator
 * @details function to unmap and close a shared memory 
 * @param shmem sharedMemory that should be closed
 * @return 0 if munmap and close were successfull
 * @return -1 if munmap or close was not successfull
**/
static int generatorCloseSharedMemory(sharedMemory* shmem){
    int returnValue = 0;    
    returnValue = MIN(returnValue, munmap(shmem, sizeof(*shmem)));
    returnValue = MIN(returnValue, close(shmfd));
    return returnValue;
}

/**
 * supervisorOpenCircularBuffer function
 * @brief opens a circular buffer as a supervisor
 * @details function to open a shared memory, semaphores and return a sharedMemory that will function as a circularBuffer
 * initializes the circular buffer with rd-wr position 0.
 * @return NULL if opening of semaphores or shared memory failed
 * @return circular buffer (as shared memory struct) if opening of semaphores and shared memory were successful. 
**/
sharedMemory* supervisorOpenCircularBuffer(){
    sharedMemory* shmem;
    if((shmem = supervisorOpenSharedMemory())==NULL){
        return NULL;
    }
    if(supervisorOpenSemaphores()==-1){
        supervisorCloseSharedMemory(shmem);
        return NULL;
    }
    shmem->rd_pos = 0;
    shmem->wr_pos = 0;
    return shmem;
}

/**
 * generatorOpenCircularBuffer function
 * @brief opens a circular buffer as a generator
 * @details function to open a shared memory, semaphores and return a sharedMemory that will function as a circularBuffer
 * @return NULL if opening of semaphores or shared memory failed
 * @return circular buffer (as shared memory struct) if opening of semaphores and shared memory were successful. 
**/
sharedMemory* generatorOpenCircularBuffer(){
    sharedMemory* shmem = generatorOpenSharedMemory();
    if(shmem ==NULL){
        return NULL;
    }
    if(generatorOpenSemaphores()==-1){
        generatorCloseSharedMemory(shmem);
        return NULL;
    }
    return shmem;
}

/**
 * readCircularBuffer function
 * @brief read data from the circular buffer 
 * @details function for the generator to read from the circular buffer
 * @param shmem circular buffer to which data will be written 
 * @return NULL if sem_wait(semUsed) returns -1 (blocking)
 * @return char* to data-entry read from circular buffer
**/
char* readCircularBuffer(sharedMemory* shmem){
    int size = 64;
    char* data = malloc(sizeof(char)*size);
    for(int i =0;i<0;i++){
        if(sem_wait(semUsed)==-1){
            free(data);
            return NULL;
        }
        if(i==size){
            size = size<<1;
            data = realloc(data, size);
        }
        data[i] = shmem->buffer[shmem->rd_pos];
        shmem->rd_pos = (((shmem->rd_pos)+1)%1024);
        sem_post(semFree);
        if(data[i]=='\0'){
            break;
        } 
    }
    return data;    
}

/**
 * writeCircularBuffer function
 * @brief write data to a circular buffer
 * @details function for a sever to write to a circular buffer 
 * @param shmem circular buffer to which data will be written
 * @return -1 if semWait or semFree blocks or the circular buffer is terminated
 * @return 0 if writing is successful.
**/
int writeCircularBuffer(sharedMemory* shmem, char* data){
    if(sem_wait(semWait)==-1){
        return -1;
    }
    for(int i = 0; shmem->terminate != true; i++){
        if(sem_wait(semFree)==-1){
            sem_post(semWait);
            return -1;
        }
        char c = data[i];
        shmem->buffer[shmem->wr_pos] = c;
        shmem->wr_pos = (((shmem->wr_pos)+1)%1024);
        sem_post(semUsed);
        if(data[i]=='\0'){
            break;
        } 
    }
    sem_post(semWait);
    if(shmem->terminate){
        return -1;
    }
    return 0;
}

/**
 * supervisorCloseCircularBuffer function
 * @brief closes a circular buffer as a supervisor
 * @details function to close shared memory and semaphores used by the circular buffer
 * @return -1 if closing the shared memory or semaphores failed
 * @return 0 if closing the shared memory and semaphores were successful. 
**/
int supervisorCloseCircularBuffer(sharedMemory* shmem){
    int returnValue = 0;  
    sem_post(semFree);
    shmem->terminate = true;
    returnValue = MIN(returnValue, supervisorCloseSharedMemory(shmem));
    returnValue = MIN(returnValue, supervisorCloseSemaphores());
    return returnValue;
}

/**
 * generatorCloseCircularBuffer function
 * @brief closes a circular buffer as a generator
 * @details function to close shared memory and semaphores used by the circular buffer
 * @return -1 if closing the shared memory or semaphores failed
 * @return 0 if closing the shared memory and semaphores were successful. 
**/
int generatorCloseCircularBuffer(sharedMemory* shmem){
    int returnValue = 0;  
    sem_post(semFree);
    shmem->terminate = true;
    returnValue = MIN(returnValue, generatorCloseSharedMemory(shmem));
    returnValue = MIN(returnValue, generatorCloseSemaphores());
    return returnValue;
}




