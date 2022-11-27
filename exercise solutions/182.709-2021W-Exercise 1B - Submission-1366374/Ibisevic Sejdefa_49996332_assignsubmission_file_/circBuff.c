/**
 * @file circBuff.c
 * @author Sejdefa Ibisevic <e11913116@student.tuwien.aca.at>
 * @brief The circBuff.h file represents h file for shared memory for helping execution of generators and supervisor
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
**/

#include "circBuff.h"
#include <fcntl.h> /* For O_* constants */

char *myprogram;
int myshmfd;


sem_t *free_sem;
sem_t *used_sem;
sem_t *mutex_sem;

/**
 * @brief Function that opens shared memory
 * @brief Function takes 0 or 1 as a flag, depends if opens existing or new shared memory. Result is stored in shared
 * memory descriptor. If shm_open returns negative value, then function retunrs NULL as an error.
 * On shm_open success and if new shared memory is created, ftrunicate is called to set the size of new shared memory.
 * In case of unsuccessful setting of buffer size, shared memory is closed and unlinked.  
 * @param type - accepts flag 0 for opening existing shared memory and 1 for creating new shared memory
 * @return buffer - returns created buffer on succes and NULL
 */
static buffer *open_shm (int type) {

    //type = false for exitsting buff, type = true for new buff
    //O_CREAT : create the file if it does not
    //O_EXCL : fail if the file already exists exist
    if (type == 0)  {
        myshmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    } else if (type == 1) {
        myshmfd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, 0600);
    }

    if (myshmfd == -1)
    {
        return NULL;
    }

    //setting size of buffer for new buffer
    if(type) {
        //returns 0 on success, -1 on error
        if (ftruncate(myshmfd, sizeof(buffer)) < 0)
        {
            fprintf(stderr, "%s %d: ERROR. Failed to set size of shared memory.\n", myprogram, (int)getpid());

            if (close(myshmfd) == -1) {
                fprintf(stderr, "%s %d: ERROR. Failed to close shared memory.\n", myprogram, (int)getpid());
            }
            if (shm_unlink(SHM_NAME) == -1) {
                fprintf(stderr, "%s %d: ERROR. Failed to unlink shared memory.\n", myprogram, (int)getpid());
            }

            return NULL;
        }
    }

    //mapping buffer
    buffer *buff;
    buff = mmap(NULL, sizeof(*buff), PROT_READ | PROT_WRITE, MAP_SHARED, myshmfd, 0);

    if (buff == MAP_FAILED)
    {
        fprintf(stderr, "%s %d: ERROR. Failed to map shared memory.\n", myprogram, (int)getpid());

        if (close(myshmfd) == -1) {
            fprintf(stderr, "%s %d: ERROR. Failed to close shared memory.\n", myprogram, (int)getpid());
        }
        if (shm_unlink(SHM_NAME) == -1) {
            fprintf(stderr, "%s %d: ERROR. Failed to unlink shared memory.\n", myprogram, (int)getpid());
        }
        return NULL; //error
    }

    return buff;
}
/**
 * @brief Function that closes shared memory
 * @details Function that takes care of unmapping, closing and unlinking of shared memory
 * @param buff - shared memory buffer that needs to be closed
 * @param type - type 0 or 1, in case of type 1, shm needs to be unlinked too
 * @return result - returns 0 on succes and -1 on error
 */
static int close_shm(buffer *buff, int type)
{
    int result = 0;
    if (munmap(buff, sizeof(*buff)) == -1)
    {
        fprintf(stderr, "%s %d: ERROR. Failed to unmap shared memory.\n", myprogram, (int)getpid());
        result = -1; //error
    }

    if (close(myshmfd) == -1)
    {
        fprintf(stderr, "%s %d: ERROR. Failed to close shared memory.\n", myprogram, (int)getpid());
        result = -1; //error
    }

    if (type)
    {
        if (shm_unlink(SHM_NAME) == -1)
        {
            fprintf(stderr, "%s %d: ERROR. Failed to unlink shared memory.\n", myprogram, (int)getpid());
            result = -1; //error
        }
    }

    return result;
}
/**
 * @brief Function that opens semaphores
 * @details Function uses dem_open function to open all three semaphores (free, used and mutex).
 * If sem_open returnes SEM_FAILED on failure, then all previous semaphores are being closed and unlink, and -1 is returned.
 * It recgnises between type 0 for existing semaphore and type 1 for new semaphore
 * @param type - flag that determines if existing (0) or new (1) semaphore needs to be open/created
 * @return int returns 0 on success and -1 on error
 */
static int open_sem(int type)
{
    //type = false for exitsting sem, type = true for new sem
    /* create a new named semaphore */
    if (type)
    {
        //open free semaphore
        free_sem = sem_open(FREE_SEM, O_CREAT | O_EXCL, 0600, SHM_MAX_DATA);
        if (free_sem == SEM_FAILED)
        {
            fprintf(stderr, "%s %d: ERROR. Failed to open %s .\n", myprogram, (int)getpid(), FREE_SEM);
            return -1;
        }

        //open used semaphore
        used_sem = sem_open(USED_SEM, O_CREAT | O_EXCL, 0600, 0);

        if (used_sem == SEM_FAILED)
        {
            //close and unling FREE semaphore
            fprintf(stderr, "%s %d: ERROR. Failed to open %s .\n", myprogram, (int)getpid(), USED_SEM);

            if (sem_close(free_sem) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to close %s .\n", myprogram, (int)getpid(), FREE_SEM);
            }
            if (sem_unlink(FREE_SEM) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to unlink %s .\n", myprogram, (int)getpid(), FREE_SEM);
            }

            return -1;
        }

        //open mutex semaphore
        mutex_sem = sem_open(MUTEX_SEM, O_CREAT | O_EXCL, 0600, 1);
        if (mutex_sem == SEM_FAILED)
        {
            fprintf(stderr, "%s %d: ERROR. Failed to open %s .\n", myprogram, (int)getpid(), MUTEX_SEM);
            //close and unling FREE and USED semaphore
            if (sem_close(free_sem) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to close %s .\n", myprogram, (int)getpid(), FREE_SEM);
            }
            if (sem_close(used_sem) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to close %s .\n", myprogram, (int)getpid(), USED_SEM);
            }

            if (sem_unlink(FREE_SEM) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to unlink %s .\n", myprogram, (int)getpid(), FREE_SEM);
            }
            if (sem_unlink(USED_SEM) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to unlink %s .\n", myprogram, (int)getpid(), USED_SEM);
            }

            return -1;
        }
    }
    /* open an existing named semaphore */
    else
    {
        free_sem = sem_open(FREE_SEM, 0);
        if (free_sem == SEM_FAILED)
        {
            fprintf(stderr, "%s %d: ERROR. Failed to open %s .\n", myprogram, (int)getpid(), FREE_SEM);
            return -1;
        }

        used_sem = sem_open(USED_SEM, 0);
        if (used_sem == SEM_FAILED)
        {
            fprintf(stderr, "%s %d: ERROR. Failed to open %s .\n", myprogram, (int)getpid(), USED_SEM);
            if (sem_close(free_sem) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to close %s .\n", myprogram, (int)getpid(), FREE_SEM);
            }
            return -1;
        }

        mutex_sem = sem_open(MUTEX_SEM, 0);
        if (mutex_sem == SEM_FAILED)
        {
            fprintf(stderr, "%s %d: ERROR. Failed to open %s .\n", myprogram, (int)getpid(), MUTEX_SEM);
            if (sem_close(free_sem) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to close %s .\n", myprogram, (int)getpid(), FREE_SEM);
            }
            if (sem_close(used_sem) < 0) {
                fprintf(stderr, "%s %d: ERROR. Failed to close %s .\n", myprogram, (int)getpid(), USED_SEM);
            }
            return -1;
        }
    }

    return 0;
}
/**
 * @brief Function that takes care of closing semaphores
 * 
 * @param type - flag 0 or 1, in case of 1 unlinking of semaphore is also needed
 * @return int - returns 0 on succes and -1 on error
 */
static int close_sem(int type)
{
    int result = 0;

    if (sem_close(free_sem) == -1 || sem_close(used_sem) == -1 || sem_close(mutex_sem) == -1)
    {
        fprintf(stderr, "%s %d: ERROR. Failed to close semaphore .\n", myprogram, (int)getpid());
        result = -1;
    }

    if (type)
    {
        if (sem_unlink(FREE_SEM) == -1 || sem_unlink(USED_SEM) == -1 || sem_unlink(MUTEX_SEM) == -1)
        {
            fprintf(stderr, "%s %d: ERROR. Failed to unlink semaphore .\n", myprogram, (int)getpid());
            result = -1;
        }
    }

    return result;
}
/**
 * @brief Help function used to open buffer and semaphores
 * @details New pointer to buffer is created. Calls opens_shm to create new shared memory
 * or to open exiting one, according to the flag. If buffer returns NULL, then error is reported. 
 * If new shared memory is created, positions for read and write are initialized to 0.
 * Function also creates or opens semaphores.
 * @param type - 0 or 1 flag, depends if existing buffer is open or new created
 * @return buffer - returns pointer to opened buffer
 */
buffer *open_buff(int type)
{
    buffer *buff = open_shm(type);
    if (buff == NULL)
    {
        return NULL;
    }

    if (open_sem(type) == -1)
    {
        close_shm(buff, type);
        return NULL;
    }

    if (type == 1)
    {
        buff->pos_read = 0;
        buff->pos_write = 0;
        buff->exit = false;
    }

    return buff;
}
/**
 * @brief Function that is used to read from shared memory
 * @details Function first does initial allocation of resources in which read content whill be stored.
 * In each iteration function lockes used semapore and stores character from circular buffer at read position 
 * to data array. Function then unlockes free semaphore. If data array is to small rellocation is done to double of initial capacity.
 * If end of string is reached, iteration exits. 
 * @param buff - pointer to shared memory from which data is being read
 * @return data - returns pointer to array of characters where data is stored
 */
char *read_buff(buffer *buff)
{
    int data_capacity = 50;
    char *data = malloc(sizeof(char) * data_capacity);
    int i = 0;
    while(true)
    {
        if (sem_wait(used_sem) == -1)
        {
            free(data);
            return NULL;
        }
        if (i == data_capacity)
        {
            //reallocating memory in case it is needed
            data_capacity *= 2;
            data = realloc(data, data_capacity);
        }

        data[i] = buff->circular_buffer[buff->pos_read];
        buff->pos_read += 1;
        buff->pos_write %= SHM_MAX_DATA;
        sem_post(free_sem);

        //exit if end of string
        if (data[i] == 0)
        {
            break;
        }
        i++;
    }
    return data;
}
/**
 * @brief Function that writes data to shared memory, with the help of semaphores
 * @details Function lockes mutex and free semaphores.While data of i is not 0, character c is taken from data array and stored in circular_buffer
 * on write position. Write position is then increased. Before each iteration free semaphore is locked and after iteration used semaphore is unlocked.
 * After all data is written to buffer, mutex semaphore is unlocked. 
 * @param buff - pointer to shared memory buffer in which data is written
 * @param data - data pointer to array of characters that needs to be written to buffer
 * @return result - returns 0 on succes and -1 on error 
 */
int write_buff(buffer *buff, const char *data)
{
    //strings are null terminated
    if (sem_wait(mutex_sem) == -1)
    {
        return -1;
    }

    for (int i = 0; buff->exit == false; i++)
    {
        if (sem_wait(free_sem) == -1)
        {
            sem_post(mutex_sem);
            return -1;
        }
        char c = data[i];
        buff->circular_buffer[buff->pos_write] = c;
        buff->pos_write += 1;
        buff->pos_write %= SHM_MAX_DATA;
        sem_post(used_sem);

        if (data[i] == 0)
        {
            break;
        }
    }
    sem_post(mutex_sem);

    if (buff->exit)
    {
        return -1;
    }

    return 0;
}
/**
 * @brief Function that closes buffer and semaphores and invokes buffer flag exit to true
 * @details Function sets exit flag to true, and calls sem_post over free semaphore. 
 * Then, it closes shared memory and all semaphores.
 * @param buff - pointer to buffer that needs to be closed
 * @param type - flag 0 or 1 depending if shared memory needs to be unlinked or not 
 * @return result - returns 0 on success and -1 of error
 */
int close_buff(buffer *buff, int type)
{
    int result = 0;
    sem_post(free_sem);
    buff->exit = true;

    if (close_shm(buff, type) == -1)
    {
        result = -1;
    }
    if (close_sem(type) == -1)
    {
        result = -1;
    }
    return result;
}


