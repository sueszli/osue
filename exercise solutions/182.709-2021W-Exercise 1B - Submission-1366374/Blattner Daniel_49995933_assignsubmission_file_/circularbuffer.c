/**
 * @file circularbuffer.c
 * @author Daniel Blattner <e12020646@students.tuwien.ac.at>
 * @date 09.11.2021
 *
 * @brief Implementation of the circular buffer module.
 *
**/

#include "circularbuffer.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>

/**
 * @details Visibility restricted to circularbuffer module.
**/
void printErrorMsg(char *progName, char *msg)
{
    fprintf(stderr, "[%s] ERROR: %s: %s\n", progName, msg, strerror(errno));
}

/**
 * @details Visibility restricted to circularbuffer module.
**/
void printErrorMsgExit(char *progName, char *msg)
{
    printErrorMsg(progName,msg);
    exit(EXIT_FAILURE);
}

/**
 * @details Visibility restricted to circularbuffer module.
**/
void printDebugMsg(char *progName, char *msg)
{
    fprintf(stdout, "[%s] %s\n", progName, msg);
}

/**
 * @brief This function creates/opens the shared memory. Then it maps the memory and
 * finally closes the shared memory file descriptor. 
 * @details If the isCreator is true, the shared memory will be created and truncated.
 * @param progName The name of the programm as defined in argv[0]
 * @param buffer Pointer to the struct, where the shared memory pointer is saved.
 * @param isCreator If true create shared memory, else open it. 
**/
static void createSharedMemory(char *progName, SharedResource *buffer, bool isCreator)
{
    assert(buffer != NULL);

    //Create/Open shared memory object
    int oflag = (isCreator) ? O_RDWR | O_CREAT : O_RDWR;
    int shmfd = shm_open(SHM_NAME, oflag, 0600);
    if(shmfd == -1){
        printErrorMsgExit(progName, "Could not create/open shared memory object");
    }

    //Truncate shared memory object
    if(isCreator){
        if(ftruncate(shmfd, sizeof(CircularBuffer)) == -1){
            close(shmfd);
            shm_unlink(SHM_NAME);
            printErrorMsgExit(progName, "Could not truncate shared memory");   
        }
    }

    //Map shared memory
    buffer->buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(buffer->buffer == MAP_FAILED){
        close(shmfd);
        shm_unlink(SHM_NAME);
        printErrorMsgExit(progName, "Could not map shared memory");
    }

    close(shmfd);
}

/**
 * @brief This function unmap (and unlink) the shared memory in the struct buffer.
 * @details On an error this function will print and error message to stderr, 
 * however it will not exit the program.
 * @param progName The name of the programm as defined in argv[0]
 * @param buffer Pointer to the struct, where the shared memory pointer is saved.
 * @param isCreator If true the shared memory will be unlinked, else just unmaped.
 * @return Returns true if no error happended, else false.
**/
static bool destroySharedMemory(char *progName, SharedResource *buffer, bool isCreator)
{
    assert(buffer != NULL);

    bool error = false;
    //Unmap shared memory
    if(munmap((void *)buffer->buffer, sizeof(CircularBuffer)) == -1){
        error |= true;
        printErrorMsg(progName, "Could not close memory map");
    }

    //Unlink shared memory
    if(isCreator){
        if(shm_unlink(SHM_NAME) == -1){
            error |= true;
            printErrorMsg(progName, "Could not unlink shared memory");
        }
    }

    return error;
}

/**
 * @brief This function creates/opens all semaphores needed for the circular
 * buffer. Furthermore if an error occurs, it will close/unmap all shared 
 * resources and exits with an error message. 
 * @param progName The name of the programm as defined in argv[0]
 * @param sem Pointer to the struct, where the semaphore pointer are saved.
 * @param isCreator If true the semaphore are created, else opened. 
**/
static void createSemaphores(char *progName, SharedResource *sem, bool isCreator)
{
    assert(sem != NULL);

    //Open read and write semaphores
    if(isCreator){
        sem->readrdy = sem_open(SEM_READRDY, O_CREAT | O_EXCL, 0600, 0);
        sem->writerdy = sem_open(SEM_WRITERDY, O_CREAT | O_EXCL, 0600, MAX_DATA);
        sem->writemutex = sem_open(SEM_WRITEMUTEX, O_CREAT | O_EXCL, 0600, 1);
    }
    else{
        sem->readrdy = sem_open(SEM_READRDY, 0);
        sem->writerdy = sem_open(SEM_WRITERDY, 0);
        sem->writemutex = sem_open(SEM_WRITEMUTEX, 0);
    }

    if(sem->readrdy == SEM_FAILED){
        munmap((void *)sem->buffer, sizeof(CircularBuffer));
        sem_close(sem->writerdy);
        sem_close(sem->writemutex);
        printErrorMsgExit(progName, "Could not open/create read semaphores");
    }
    if(sem->writerdy == SEM_FAILED){
        munmap((void *)sem->buffer, sizeof(CircularBuffer));
        sem_close(sem->readrdy);
        sem_close(sem->writemutex);
        printErrorMsgExit(progName, "Could not open/create write semaphores");
    }
    if(sem->writemutex == SEM_FAILED){
        munmap((void *)sem->buffer, sizeof(CircularBuffer));
        sem_close(sem->readrdy);
        sem_close(sem->writerdy);
        printErrorMsgExit(progName, "Could not open/create write mutex semaphores");
    }
}

/**
 * @brief This function closes/unlinks all semaphores in the struct shared resources.
 * @details On an error this function will print and error message to stderr, 
 * however it will not exit the program.
 * @param progName The name of the programm as defined in argv[0]
 * @param sem Pointer to the struct, where the semaphore pointer are saved.
 * @param isCreator If true the semaphores will be unlinked in addition to getting
 * closed.
 * @return Returns true if no error happended, else false.
**/
static bool destroySemaphores(char *progName, SharedResource *sem, bool isCreator)
{
    assert(sem != NULL);

    bool error = false;
    //Close read and write semaphores
    if(sem_close(sem->readrdy) == -1){
        error |= true;
        printErrorMsg(progName, "Could not close read semaphores");
    }
    if(sem_close(sem->writerdy) == -1){
        error |= true;
        printErrorMsg(progName, "Could not close write semaphores");
    }
    if(sem_close(sem->writemutex) == -1){
        error |= true;
        printErrorMsg(progName, "Could not close write semaphores");
    }
    
    //Unlink semaphores
    if(isCreator){
        if(sem_unlink(SEM_READRDY) == -1){
            error |= true;
            printErrorMsg(progName, "Could not unlink read semaphores");
        }
        if(sem_unlink(SEM_WRITERDY) == -1){
            error |= true;
            printErrorMsg(progName, "Could not unlink write semaphores");
        }
        if(sem_unlink(SEM_WRITEMUTEX) == -1){
            error |= true;
            printErrorMsg(progName, "Could not unlink write semaphores");
        }
    }

    return error;
}

/**
 * @details Visibility restricted to circularbuffer module.
**/
void initSharedResource(char *progName, SharedResource *shr, bool isCreator)
{
    assert(shr != NULL);

    //Create shared memory and semaphores
    createSharedMemory(progName, shr, isCreator);
    createSemaphores(progName, shr, isCreator);
}

/**
 * @details Visibility restricted to circularbuffer module.
**/
void destroySharedResource(char *progName, SharedResource *shr, bool isCreator)
{
    assert(shr != NULL);
    
    bool error = false;
    //Close/unlink shared memory and semaphores
    error |= destroySharedMemory(progName, shr, isCreator);
    error |= destroySemaphores(progName, shr, isCreator);

    if(error) exit(EXIT_FAILURE);
}

/**
 * @details Visibility restricted to circularbuffer module.
**/
bool writeEdgeArray(char *progName, SharedResource *shr, EdgeArray writeData)
{
    assert(shr != NULL);

    //Wait for free slot in circular buffer
    while(sem_wait(shr->writerdy) < 0){
        if(errno != EINTR){
            printErrorMsg(progName, "Could not write on shared memory: write semaphore failed");
            return false;
        }
        if(shr->buffer->termGen == 1) return false;
    }
    //Wait for write mutex
    while(sem_wait(shr->writemutex) < 0){
        if(errno != EINTR){
            printErrorMsg(progName, "Could not write on shared memory: write mutex semaphore failed");
            return false;
        }
        if(shr->buffer->termGen == 1) return false;
    }
    //Generator signals to be terminated
    if(shr->buffer->termGen == 1) {
        sem_post(shr->writerdy);
        sem_post(shr->writemutex);
        return false;
    }

    //Write data to next free slot
    unsigned int index = shr->buffer->writeInd;
    memcpy((void *)&(shr->buffer->data[index]), &writeData, sizeof(EdgeArray));

    //Move write index to next free slot
    index++;
    index %= MAX_DATA;
    shr->buffer->writeInd = index;

    //Free write mutex
    if(sem_post(shr->writemutex) == -1){
        shr->buffer->writeInd = (index == 0) ? MAX_DATA-1 : index--;
        printErrorMsg(progName, "Could not write on shared memory: write mutex semaphore failed");
        return false;
    }
    //Signal new data on circular buffer
    if(sem_post(shr->readrdy) == -1){
        shr->buffer->writeInd = (index == 0) ? MAX_DATA-1 : index--;
        printErrorMsg(progName, "Could not write on shared memory: read semaphore failed");
        return false;
    }

    return true;
}

/**
 * @details Visibility restricted to circularbuffer module.
**/
bool readEdgeArray(char *progName, SharedResource *shr, EdgeArray *readData)
{
    assert(shr != NULL);
    assert(readData != NULL);

    readData->len = DATA_LIMIT;
    //Wait for written slot in circular buffer
    while(sem_wait(shr->readrdy) < 0){
        if(errno != EINTR){
            printErrorMsg(progName, "Could not read from shared memory: read semaphore failed");
            return false;
        }
        if(shr->buffer->termGen == 1) return false;
    }

    //Read data from written slot
    unsigned int index = shr->buffer->readInd;
    memcpy(readData, (void *)&(shr->buffer->data[index]), sizeof(EdgeArray));

    //Move read index to next written slot
    index++;
    index %= MAX_DATA;
    shr->buffer->readInd = index;

    //Signal data has been read from circular buffer
    if(sem_post(shr->writerdy) == -1){
        shr->buffer->readInd = (index == 0) ? MAX_DATA-1 : index--;
        printErrorMsg(progName, "Could not read from shared memory: write semaphore failed");
        return false;
    }

    return true;
}