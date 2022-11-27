/**
 * Shared memory module for generators
 * @file shmgen.c
 * @author Marvin Flandorfer, 52004069
 * @date 05.11.2021
 * 
 * @brief Implementation of the shared memory module for generators.
 */

#include "shmgen.h"
#include "misc.h"

shm_t *open_shared_memory(void){
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);           /**< File descriptor for the shared memory object*/
    if(shmfd < 0){
        error_message("shm_open");
        return NULL;
    }
    shm_t *shm;                                             /**< Pointer that points to the shared memory object*/
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
    return shm;
}

int cleanup_shared_memory(shm_t *shm){
    if(munmap(shm, sizeof(*shm)) < 0){
        error_message("munmap");
        return -1;
    }
    return 0;
}