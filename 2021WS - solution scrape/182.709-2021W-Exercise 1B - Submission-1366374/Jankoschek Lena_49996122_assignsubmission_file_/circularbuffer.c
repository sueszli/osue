/**
 * @file circularbuffer.c
 * @author Lena Jankoschek - 12019852
 * @brief implements a circular buffer for 3coloring program
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>

#include "circularbuffer.h"


/**
 * @brief semaphore names definitions,, shared memory name definition
 * 
 */
//semaphores
#define SEM_FREE_SPACE "/12019852_free_space_semaphore"
#define SEM_USED_SPACE "/12019852_used_space_semaphore"
#define SEM_WRITE "/12019852_write_semaphore"

//shared memory
#define SHM_NAME "/12019852_shared_memory_name"



/**
 * @brief this function creates and initializes the semaphores of a circularbuffer
 * @details this function creates and initializes semaphores of a circularbuffer. It is usually called by the supervisor. 
 * It creates the semaphore for the free space of the buffer, the semaphore for the used space of the buffer and the semaphore
 * which allows to write (sem_write). If an error occures, the resources are freed and the already opened and created semaphores are closed.
 * @param circbuf - pointer to the circularbuffer of which the semaphores should be created.
 * @return int - the return code which which the function exits. 0 represents that everything is okay. If the return code is not 0, than an error occured
 */
static int create_semaphores(struct circularbuffer *circbuf) {
    //initialize free space semaphore with buffer length
    circbuf->sem_free_space = sem_open(SEM_FREE_SPACE, O_CREAT | O_EXCL, 0600, MAX_DATA);
    if(circbuf->sem_free_space == SEM_FAILED) {
        return -1;
    }
    //initialize used space semaphore with 0 (no used space at the beginning)
    circbuf->sem_used_space = sem_open(SEM_USED_SPACE, O_CREAT | O_EXCL, 0600, 0);
    if(circbuf->sem_used_space == SEM_FAILED) {
        if(sem_close(circbuf->sem_free_space) == -1) {
            free(circbuf);
            return -1;
        }
        if(sem_unlink(SEM_FREE_SPACE) == -1) {
            free(circbuf);
            return -1;
        }
        free(circbuf);
        return -1;
    }
    //initialize write semaphore with 1 -> only one generator can write at same time 
    //if generator is currently writing -> semaphore = 0, if semaphore is free -> semaphore = 1
    circbuf->sem_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);
    if(circbuf->sem_write == SEM_FAILED) {
        if(sem_close(circbuf->sem_free_space) == -1) {
            free(circbuf);
            return -1;
        }
        if(sem_unlink(SEM_FREE_SPACE) == -1) {
            free(circbuf);
            return -1;
        }
        if(sem_close(circbuf->sem_used_space) == -1) {
            free(circbuf);
            return -1;
        }
        if(sem_unlink(SEM_USED_SPACE) == -1) {
            free(circbuf);
            return -1;
        }
        free(circbuf);
        return -1;
    }
    //everything okay
    return 0;
}


/**
 * @brief this function opens the semaphores of a circularbuffer
 * @details this function opens already created semaphores of a circularbuffer. It is usually called by a generator. 
 * It opens the semaphore for the free space of the buffer, the semaphore for the used space of the buffer and the semaphore
 * which allows a generator to write (sem_write). If an error occures, the resources are freed and the already opened semaphores are closed.
 * @param circbuf - pointer to the circularbuffer of which the semaphores should be opened.
 * @return int - the return code which which the function exits. 0 represents that everything is okay. If the return code is not 0, than an error occured
 */
static int open_semaphores(struct circularbuffer *circbuf) {
    //open free space semaphore with buffer length
    circbuf->sem_free_space = sem_open(SEM_FREE_SPACE, 0);
    if(circbuf->sem_free_space == SEM_FAILED) {
        free(circbuf);
        return -1;
    }
    //open used space semaphore with 0 (no used space at the beginning)
    circbuf->sem_used_space = sem_open(SEM_USED_SPACE, 0);
    if(circbuf->sem_used_space == SEM_FAILED) {
        if(sem_close(circbuf->sem_free_space) == -1) {
            free(circbuf);
            return -1;
        }
        free(circbuf);
        return -1;
    }
    //open write semaphore with 1 -> only one generator can write at same time 
    //if generator is currently writing -> semaphore = 0, if semaphore is free -> semaphore = 1
    circbuf->sem_write = sem_open(SEM_WRITE, 0);
    if(circbuf->sem_write == SEM_FAILED) {
        if(sem_close(circbuf->sem_free_space) == -1) {
            free(circbuf);
            return -1;
        }
        if(sem_close(circbuf->sem_used_space) == -1) {
            free(circbuf);
            return -1;
        }
        free(circbuf);
        return -1;
    }
    //everything okay
    return 0;
}


/**
 * @brief this function creates or opens (coo) the shared memory myshm. 
 * @details this function creates a shared memory, if it's called the first time (by the supervisor), or opens the already created memory if called then (by generators).
 * It opens the shared memory, set its size and maps it. If an error occures, the return code is -1 and the resources are freed.
 * @param circbuf - pointer to the circularbuffer where a shared memory should be created or opened
 * @return int - the return code which which the function exits. 0 represents that everything is okay. If the return code is not 0, than an error occured
 */
static int coo_sharedmemory(struct circularbuffer *circbuf) {
    //create or open the shared memory object
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT,  0600);
    if(shmfd == -1) {
        free(circbuf);
        return -1;
    }
    //set the size of the shared memory
    if(ftruncate(shmfd, sizeof(struct myshm)) < 0) {
        if(close(shmfd) == -1) {
            free(circbuf);
            return -1;
        }
        free(circbuf);
        return -1;
    }

    //map the shared memory object
    circbuf->shm = mmap(NULL, sizeof(struct myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(circbuf->shm == MAP_FAILED) {
        if(close(shmfd) == -1) {
            free(circbuf);
            return -1;
        }
        free(circbuf);
        return -1;
    }
    if(close(shmfd) == -1) {
        free(circbuf);
        return -1;
    }

    //everything okay
    return 0;
}


/**
 * @brief this function creates or opens (coo) a circularbuffer and returns the buffer.
 * @details depending on who called this function, a circular buffer is created or opened. 
 * If the supervisor calles this function (caller = 's') the buffer and with it the 3 semaphore and the shared memory. 
 * If a generator calls this function (caller = 'g') it returns the buffer, if it was already created by the 
 * supervisor, otherwise an error is thrown. It also opens the semaphores and the shared memory.
 * @param caller - a char which represents from which program the function was called. 's' = function was called from superviser, 
 * 'g' = function was called from generator.
 * @return struct circularbuffer* - the circularbuffer which was created (caller = s) or opened (caller = g) by the function
 */
struct circularbuffer *cb_coo(char caller) {
    //caller is either s (supervisor) or g (generator)
    assert(caller == 's' || caller == 'g');

    //allocates space for circular buffer
    struct circularbuffer *circbuf = malloc(sizeof(struct circularbuffer));
    
    if(circbuf == NULL) {
        return NULL;
    }
    
    //if supervisor called the program -> initialize state, read position and write position
    if(caller == 's') {
        //create semaphores
        if(create_semaphores(circbuf) != 0) {
            free(circbuf);
            return NULL;
        }

        //create or open shared memory
        if(coo_sharedmemory(circbuf) == -1) {
            free(circbuf);
            return NULL;
        }

        circbuf->shm->read_pos = 0;
        circbuf->shm->write_pos = 0;
        circbuf->shm->state = 0;
    } else {
        //open semaphores
        if(open_semaphores(circbuf) != 0) {
            free(circbuf);
            return NULL;
        }
        //create or open shared memory
        if(coo_sharedmemory(circbuf) == -1) {
            free(circbuf);
            return NULL;
        }
    }    
    return circbuf;
}


/**
 * @brief this function closes the circularbuffer and all its resources.
 * @details the function frees the circularbuffer, the semaphores und unmaps the shared memory. 
 * If the supervisor is the caller of the functon (caller = 's'), then it also unlinks the semaphores and the shared memory.
 * @param circubuf - pointer to the circularbuffer that should be closed
 * @param caller - a char which represents from which program the function was called. 's' = function was called from superviser, 
 * 'g' = function was called from generator.
 * @return int - the return code which which the function exits. 0 represents that everything is okay. If the return code is not 0, than an error occured
 */
int cb_close(struct circularbuffer *circbuf, char caller) {
    //either the supervisor (s) or a generator (g) called the function
    assert(caller == 's' || caller == 'g');

    //remove mappings of the shared memory
    if(caller == 's') {
        circbuf->shm->state = 1;
    }
    if (munmap(circbuf->shm, sizeof(*(circbuf->shm))) == -1) {
        free(circbuf);
        return -1;
    }

    //close semaphores
    if(sem_close(circbuf->sem_free_space) == -1) {
        free(circbuf);
        return -1;
    }
    if(sem_close(circbuf->sem_used_space) == -1) {
        free(circbuf);
        return -1;
    }
    if(sem_close(circbuf->sem_write) == -1) {
        free(circbuf);
        return -1;
    }

    //supervisor is calling the function -> unlink semaphores & free circular buffer
    if(caller == 's') {
        if(sem_unlink(SEM_FREE_SPACE) == -1) {
            free(circbuf);
            return -1;
        }
        if(sem_unlink(SEM_USED_SPACE) == -1) {
            free(circbuf);
            return -1;
        }
        if(sem_unlink(SEM_WRITE) == -1) {
            free(circbuf);
            return -1;
        }
        if (shm_unlink(SHM_NAME) == -1) {
            free(circbuf);
            return -1;
        }
    }

     free(circbuf);

    //everythink okay
    return 0;
}


/**
 * @brief this function writes a solution into the circularbuffer
 * @details the function writes a solution into the circularbuffer. It waits until it is allowed to write (the write semaphore "is free" and there
 * is free space in the circularbuffer). Until then it blocks. If it is its turn to write, the given solution is written into the cirularbuffer
 * on the current write position.
 * @param circbuf - pointer to the circularbuffer which should be written to
 * @param sol - the solution which is written into the buffer
 * @return int - the return code which which the function exits. 0 represents that everything is okay. If the return code is not 0, than an error occured
 */
int cb_write(struct circularbuffer *circbuf, struct solution sol) {
    //wait til there is free space to write to -> decrement free space semaphore
    if (sem_wait(circbuf->sem_free_space) == -1) {
        return -1;
    }
    //wait til write semaphore is 1, then decrement write semaphore -> no one else can write in the mean time
    if (sem_wait(circbuf->sem_write) == -1) {
        return -1;
    }
    //check if the state of the circular buffer changed -> shouldn't write anymore
    if (circbuf->shm->state != 0) {
        //increment the write semaphore
        if(sem_post(circbuf->sem_write) == -1) {
            cb_close(circbuf, 'g');
            return -2;
        }
        cb_close(circbuf, 'g');
        return -2;
    }
    //set the solution at writing position
    circbuf->shm->data[circbuf->shm->write_pos] = sol;
    //increment the used space -> one more space is used
    if (sem_post(circbuf->sem_used_space) == -1) {
        return -1;
    }
    //recalculate the new writing position
    circbuf->shm->write_pos++;
    circbuf->shm->write_pos%= MAX_DATA;
    //increment the write semaphore -> someone else can write
    if (sem_post(circbuf->sem_write) == -1) {
        return -1;
    }
    return 0;
}


/**
 * @brief this function reads a solution from the circularbuffer
 * @details the function reads a solution from the circularbuffer from the current read position. It waits until there is something to read
 * from the buffer (used space semaphore is not 0). Until then it blocks. 
 * @param circbuf - pointer to the circularbuffer that should be read from
 * @return struct solution - the solution which was read from the buffer. if sol.size = -1 -> no solution in buffer
 */
struct solution cb_read(struct circularbuffer *circbuf) {
    //get solution from reading position
    struct solution sol;
    
    //decrement used space semaphore 
    if (sem_wait(circbuf->sem_used_space) == -1) {
        sol.size = -1;
        return sol;
    }

    sol = circbuf->shm->data[circbuf->shm->read_pos];

    //increment free space semaphore
    if(sem_post(circbuf->sem_free_space) == -1) {
        sol.size = -1;
        return sol;
    }

    //calculate the new reading position
    circbuf->shm->read_pos++;
    circbuf->shm->read_pos %= MAX_DATA;

    //return the solution at the reading position
    return sol;
}
