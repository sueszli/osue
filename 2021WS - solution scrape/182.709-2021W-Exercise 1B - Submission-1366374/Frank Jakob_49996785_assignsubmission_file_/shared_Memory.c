/*
*
* @file shared_Memory.c
* @author Jakob Frank (11837319)
*
* @brief implements functions to initiate the 
*        shared Memory as client and server
*
* @date 12/11/2021
*/ 


#include "circular_buffer.h"


/*
*
* @brief initializes the shared memory as a server
*
* @param fdptr a reference to the file descriptor
* @param memptr a reference to the shared memory
*/

void memInitSupervisor(int *fdptr, buf ** memptr)
{
    if ((*fdptr = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1)
    {
        perror("shm_open");
        __builtin_trap();
    }

    if(ftruncate(*fdptr, sizeof(buf)) == -1)
    {   
        fprintf(stderr,"Error truncating shared memory\n");
        exit(EXIT_FAILURE);
    }

    *memptr = mmap(NULL, sizeof(buf), PROT_READ | PROT_WRITE, MAP_SHARED, *fdptr, 0);

    if (*memptr == MAP_FAILED)
    {
        fprintf(stderr,"Error mapping shared memory\n");
        exit(EXIT_FAILURE);
    }

    if (close(*fdptr) == -1)
    {
        fprintf(stderr,"Error opening shm\n");
        exit(EXIT_FAILURE);
    }
    
    //enter crit. section
}

/*
*
* @brief initializes the shared memory as a client
*
* @param fdptr a reference to the file descriptor
* @param memptr a reference to the shared memory
*/

void memInitGenerator(int *fdptr, buf ** memptr)
{
    *fdptr = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR);

    if (*fdptr == -1)
    {
        fprintf(stderr,"Error opening shm, file doesn't exist!\n");
        exit(EXIT_FAILURE);
    }
    
    *memptr = mmap(NULL, sizeof(buf), PROT_READ | PROT_WRITE, MAP_SHARED, *fdptr, 0);

    if (*memptr == MAP_FAILED)
    {
        fprintf(stderr,"Error mapping shared memory\n");
        exit(EXIT_FAILURE);
    }

    //enter crit section

}

/*
*
* @brief closes the shared memory as a server
*
* @param fdptr a reference to the file descriptor
* @param memptr a reference to the shared memory
*/

void memClearSupervisor(int * fdptr, buf ** memptr)
{
    if (munmap(memptr, sizeof(buf)) == -1)
    {
        fprintf(stderr,"Error unmapping shared memory!\n");
        exit(EXIT_FAILURE);
    }

    if (close(*fdptr) == -1)
    {
        fprintf(stderr,"Error closing shared memory! Maybe it's already closed\n");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(SHM_NAME) == -1)
    {
        fprintf(stderr,"Error removing object\n");
        exit(EXIT_FAILURE);
    }
    
}

/*
*
* @brief closes the shared memory as a client
*
* @param fdptr a reference to the file descriptor
* @param memptr a reference to the shared memory
*/

void memClearGenerator(int * fdptr, buf ** memptr)
{
    if (munmap(memptr, sizeof(buf)) == -1)
    {
        fprintf(stderr,"Error unmapping shared memory!\n");
        exit(EXIT_FAILURE);
    }
    
    if (close(*fdptr) == -1)
    {
        fprintf(stderr,"Error closing shared memory! Maybe it's already closed\n");
        exit(EXIT_FAILURE);
    }

}