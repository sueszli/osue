#include <sys/mman.h>
#include <fcntl.h> /* for 0_ constants */
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

#define SHM_NAME "/myshm"

struct myshm{
    unsigned int state;
};

int main(int argc, char **argv){

    // create and open shared memory object
    int shmFD = shm_open(SHM_NAME, O_CREAT, 0);
    if(shmFD == -1){
        perror("Fail");
        fprintf(stderr,"%s: Creation of shared memory failed!\n", argv[0]);
        return EXIT_FAILURE;
    }

    // set size of shared memory
    if(ftruncate(shmFD, sizeof(struct myshm)) < 0){
        fprintf(stderr,"%s: Error while truncating!\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // map shared memory object
    struct myshm *myshm;
    myshm = mmap(NULL,sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
    if(myshm == MAP_FAILED){
        fprintf(stderr,"%s: Error while mapping the shared memory object!\n", argv[0]);
        return EXIT_FAILURE;
    }

    // close file descriptor
    if(close(shmFD) == -1){
        fprintf(stderr,"%s: Error while closing the file descriptor!\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // unmap shared memory
    if(munmap(myshm, sizeof(*myshm)) == -1){
        fprintf(stderr,"%s: Error while unmapping the shared memory object!\n", argv[0]);
        return EXIT_FAILURE;
    }

    // remove shared memory object:
    if (shm_unlink(SHM_NAME) == -1){
        fprintf(stderr,"%s: Error while unlinking the shared memory!\n", argv[0]);
        return EXIT_FAILURE;
    }
    

    return EXIT_SUCCESS;
}