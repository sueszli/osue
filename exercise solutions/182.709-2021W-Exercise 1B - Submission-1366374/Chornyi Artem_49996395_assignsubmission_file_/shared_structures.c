/**
 * @author Artem Chornyi. 11922295
 * @brief Structures and macros for shared objects and their namings.
 * @date 9-th November 2021 (09.11.2021)
 * 
 */


#define SHARED_STRUCTURES
#define MUTEX_NAME "/11922295_MUTEX"
#define WRITE_SEMAPHORE_NAME "/11922295_WRITE_SEMAPHORE"
#define READ_SEMAPHORE_NAME "/11922295_READ_SEMAPHORE"
#define SHARED_BUFFER_NAME "/11922295_SB"
#define SHARED_CONTROL_NAME "/11922295_SC"
#include <semaphore.h>

typedef struct
{
    // Keeps track of writePos under many children processes
    unsigned int writePos;
    // flag, that notifies both generator and supervisor, 
    // that its time to free their resources and exit.
    char recievedInterrupt;
} shared_control;

// Storage for many frequently used semaphores in both generator and supervisor.
typedef struct
{
    sem_t *mutex; 
    sem_t *writeSem;
    sem_t *readSem;
} semaphores;
