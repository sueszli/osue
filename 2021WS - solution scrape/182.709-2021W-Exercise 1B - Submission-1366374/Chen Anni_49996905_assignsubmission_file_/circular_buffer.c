#include "circular_buffer.h"
#include <assert.h>

int open_circbuf(char role)
{

    assert(role == 'c' || role == 's');

    int shmfd;

    if (role == 'c')
    {
        shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
        if (shmfd == -1)
        {
            return -1;
        }
    }

    if (role == 's')
    {

        shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
        if (shmfd == -1)
        {
            return -1;
        }

        if (ftruncate(shmfd, sizeof(*circbuf)) < 0)
        {
            return -1;
        }
    }

    circbuf = mmap(NULL, sizeof(*circbuf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (circbuf == MAP_FAILED)
    {
        return -1;
    }

    if (close(shmfd) != 0) // close file descriptor
    {
        return -1;
    }

    //initialise
    if (role == 's')
    {
        circbuf->state = 1;
        circbuf->readPos = 0;
        circbuf->writePos = 0;
    }

    return shmfd;
}

int close_circbuf(char role)
{
    assert(role == 'c' || role == 's');

    int ret_val = 0;

    if (role == 's')
    {
        //terminate c
        circbuf->state = 0;
    }

    //close semaphores
    if (sem_close(sem_free) == -1)
    {
        ret_val = -1;
    }
    if (sem_close(sem_used) == -1)
    {
        ret_val = -1;
    }
    if (sem_close(sem_write) == -1)
    {
        ret_val = -1;
    }

    if (role == 's')
    {
        //unlink semaphores
        if (sem_unlink(SEM_FREE) == -1)
        {
            ret_val = -1;
        }
        if (sem_unlink(SEM_USED) == -1)
        {
            ret_val = -1;
        }
        if (sem_unlink(SEM_WRITE) == -1)
        {
            ret_val = -1;
        }

        if (shm_unlink(SHM_NAME) == -1)
        {
            ret_val = -1;
        }
    }

    if (munmap(circbuf, sizeof(*circbuf)) == -1)
    {
        ret_val = -1;
    }

    return ret_val;
}

int init_sem(char role)
{
    assert(role == 'c' || role == 's');

    sem_free = NULL;
    sem_used = NULL;
    sem_write = NULL;

    if (role == 's')
    {

        //initialize with MAX_DATA since we can write up to MAX_DATA at the beginning
        sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_DATA);
        if (sem_free == SEM_FAILED)
        {
            return -1;
        }

        //initialize with 0 since there are no entries in the buffer at the beginning
        //indicates how many entries are ready to be read
        sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
        if (sem_used == SEM_FAILED)
        {
            return -1;
        }

        //initialize with 1 to guarantee mutual exclusion when writing to the buffer
        sem_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);
        if (sem_write == SEM_FAILED)
        {
            return -1;
        }
    }
    else //if role is c, open already existing semaphores
    {

        sem_free = sem_open(SEM_FREE, 0);
        if (sem_free == SEM_FAILED)
        {
            return -1;
        }

        sem_used = sem_open(SEM_USED, 0);
        if (sem_used == SEM_FAILED)
        {
            return -1;
        }

        sem_write = sem_open(SEM_WRITE, 0);
        if (sem_used == SEM_FAILED)
        {
            return -1;
        }
    }

    return 0;
}

struct Solution read_solution(void)
{

    struct Solution sol = {.numOfRemovedEdges = -1, .status = -1};

    // wait until there is an element to read from the buffer
    if (sem_wait(sem_used) == -1)
    {
        //return a solution where the status is set to -1 indicating an error
        return sol;
    }

    int rPos = circbuf->readPos;
    sol = circbuf->solutions[rPos];
    rPos = (rPos + 1) % MAX_DATA;
    circbuf->readPos = rPos;

    //increment to indicate that one space is free
    if (sem_post(sem_free) == -1)
    {
        //set status to -1 to indicate that an error happened
        sol.status = -1;
        return sol;
    }

    return sol;
}

int write_solution(struct Solution solution)
{

    //lock to ensure that only one generator writes at a time
    if (sem_wait(sem_write) == -1)
    {
        return -1;
    }

    //wait until there is free space to write in the buffer
    if (sem_wait(sem_free) == -1)
    {
        sem_post(sem_write);
        return -1;
    }

    //position to write to
    int wPos = circbuf->writePos;
    circbuf->solutions[wPos] = solution;
    wPos = (wPos + 1) % MAX_DATA;
    circbuf->writePos = wPos;

    //indicate that 1 space has been used
    if (sem_post(sem_used) == -1)
    {
        sem_post(sem_write);
        return -1;
    }

    //unlock write mutex
    if (sem_post(sem_write) == -1)
    {
        return -1;
    }

    return 0;
}

int get_state(void)
{
    int state = circbuf->state;

    return state;
}