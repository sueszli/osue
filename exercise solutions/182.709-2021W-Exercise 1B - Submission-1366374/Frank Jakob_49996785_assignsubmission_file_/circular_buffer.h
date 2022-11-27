/*
*
* @file circular_buffer.h
* @author Jakob Frank (11837319)
* 
* @brief extends the functions of shared_memory.h 
         and further defines methods structs and variables
*
* @date 12/11/2021
*/


#include <errno.h>
#include <semaphore.h>
#include <signal.h>

#include "shared_Memory.h"

#define BOOL_TRUE (1)
#define BOOL_FALSE (0)

#define BUF_SIZE (25)
//with a BUF_SIZE of 25 we have a max of 16*8*25+2*4 = 3208 Bytes which is below the 4kB stated in the requirements

#define SEM_FREE "sem_free"
#define SEM_USED "sem_used"
#define SEM_WRITE "sem_write"

#define WR_POS (0)

typedef struct circular_buffer
{
 shm_t mem[BUF_SIZE];
 int write_pos;
 int read_pos;
 volatile __sig_atomic_t quit;
} buf;


sem_t *sem_free;
sem_t *sem_used;
sem_t *sem_write;

/*
*   @brief  my_wait is a delay function which puts process in a wait state
*
*   @details the function runs in an endless loop until sem_wait gets called
             successfully. Unless the error in a failed sem_wait is EINTR (signal
             occurred while syscall in progress) the function will return an error.
*   @param  sem is a pointer to the semaphore which is waited on.
*
*/

void my_wait(sem_t * sem);


/*
* @brief    readMem is used to read to read from the circular buffer
*
* @details  readMem accesses the circular buffer with a read position int
*           and reads the contents into the reading process (as long as requirements see fit)
*           it then returns the size (which may be updated)
*
* @param    readPos a reference to the integer that describes the array Position to read from
* @param    bufptr  states the address of the circular buffer
* @param    size    describes the current max size to read into (gets frequently smaller)
* @param    dest    a reference to the storage, where a solution would get stored
*
*/

int readMem(int * readPos, buf * bufptr, int size, shm_t * dest);

/*
* @brief writeMem is used to write to the circular buffer
*
* @details writeMem accesses the circular buffer with a write position int
*          and writes new content at said position
*
* @param    writePos a reference to the integer that describes the array position to write to
* @param    bufptr  states the address of the circular buffer
* @param    src     references the result, which shall be written to the circular buffer at *writePos
*
*/

void writeMem(buf * bufptr, shm_t * src);

/*
*
* @brief initializes the shared memory as a server
*
* @param fdptr a reference to the file descriptor
* @param memptr a reference to the shared memory
*/

void memInitSupervisor(int *fdptr, buf ** memptr);

/*
*
* @brief initializes the shared memory as a client
*
* @param fdptr a reference to the file descriptor
* @param memptr a reference to the shared memory
*/

void memInitGenerator(int *fdptr, buf ** memptr);

/*
*
* @brief closes the shared memory as a server
*
* @param fdptr a reference to the file descriptor
* @param memptr a reference to the shared memory
*/

void memClearSupervisor(int * fdptr, buf ** memptr);

/*
*
* @brief closes the shared memory as a client
*
* @param fdptr a reference to the file descriptor
* @param memptr a reference to the shared memory
*/

void memClearGenerator(int * fdptr, buf ** memptr);
