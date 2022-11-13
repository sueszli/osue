/*
*
* @file circular_buffer.c
* @author Jakob Frank (11837319)
*
* @brief implements functions to initiate semaphores and to read and write
*        from and to the circular buffer
*
* @date 12/11/2021
*/ 


#include "circular_buffer.h"

/*
*   @brief  my_wait is a delay function which puts process in a wait state
*
*   @details the function runs in an endless loop until sem_wait gets called
             successfully. Unless the error in a failed sem_wait is EINTR (signal
             occurred while syscall in progress) the function will return an error.
*   @param  sem is a pointer to the semaphore which is waited on.
*
*/
void my_wait(sem_t * sem)
{
    while(sem_wait(sem) == -1)
    {
        if (errno == EINTR)
        {
            continue;
        }
        perror("Failed to wait on semaphore");
    }
}


/*
* @brief    readMem is used to read to read from the circular buffer
*
* @details  readMem accesses the circular buffer with a read position int
*           and read the contents into the reading process (as long as requirements see fit)
*           it then returns the size (which may be updated)
*
* @param    readPos a reference to the integer that describes the array Position to read from
* @param    bufptr  states the address of the circular buffer
* @param    size    describes the current max size to read into (gets frequently smaller)
* @param    dest    a reference to the storage, where a solution would get stored
*/
int readMem(int * readPos, buf * bufptr, int size, shm_t * dest)
{
    my_wait(sem_used);
    
    if ((*bufptr).mem[(*readPos)].size < size)
    {
        size = bufptr->mem[(*readPos)].size;
        memcpy(dest, &bufptr->mem[(*readPos) ], sizeof(shm_t));
    }

    (*readPos) ++;    
    (*readPos) = (*readPos) % BUF_SIZE;

    sem_post(sem_free);

    return size;

}

/*
* @brief writeMem is used to write to the circular buffer
*
* @details writeMem accesses the circular buffer with a write position int
*          and writes new content at said position
*
* @param    writePos a reference to the integer that describes the array position to write to
* @param    bufptr  states the address of the circular buffer
* @param    src     references the result, which shall be written to the circular buffer at *wr
*
*/
void writeMem(buf * bufptr, shm_t * src)
{
    my_wait(sem_write);
    my_wait(sem_free);
    memcpy(&bufptr->mem[(*bufptr).write_pos], src, sizeof(shm_t));

    bufptr -> write_pos ++;
    bufptr -> write_pos %= BUF_SIZE;

    sem_post(sem_used);
    sem_post(sem_write);
}

