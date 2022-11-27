/**
 * Shared memory module for the supervisor.
 * @file shmsupvis.c
 * @author Marvin Flandorfer, 52004069
 * @date 06.11.2021
 * 
 * @brief Implementation of the shared memory module for the supervisor.
 */

#include "shmsupvis.h"
#include "misc.h"

shm_t *create_shared_memory(void){
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);         /**< File descriptor for the shared memory object*/
    if(shmfd < 0){
        error_message("shm_open");
        return NULL;
    }
    if((ftruncate(shmfd, sizeof(shm_t))) < 0){
        error_message("ftruncate");
        (void) close(shmfd);
        return NULL;
    }
    shm_t *shm;                                                     /**< Pointer that points to the shared memory object*/
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(shm == MAP_FAILED){
        error_message("mmap");
        (void) close(shmfd);
        return NULL;
    }
    if((close(shmfd)) < 0){
        error_message("close");
        return NULL;
    }
    shm->state = 0;
    shm->read_pos = 0;
    shm->write_pos = 0;
    return shm;
}

int cleanup_shared_memory(shm_t *shm){
    if(munmap(shm, sizeof(*shm)) < 0){
        error_message("munmap");
        return -1;
    }
    if(shm_unlink(SHM_NAME) < 0){
        error_message("shm_unlink");
        return -1;
    }
    return 0;
}