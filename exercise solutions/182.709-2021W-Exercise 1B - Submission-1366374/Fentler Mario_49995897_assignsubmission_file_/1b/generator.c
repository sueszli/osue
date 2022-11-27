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
    if(shmFD == -1)
        fprintf(stderr,"%s: Creation of shared memory failed!", argv[0]);
  
    // map shared memory object
    struct myshm *myshm;
    myshm = mmap(NULL,sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmFD, 0);
    if(myshm == MAP_FAILED)
        fprintf(stderr,"%s: Error while mapping the shared memory object!", argv[0]);

    // close file descriptor
    if(close(shmFD) == -1)
        fprintf(stderr,"%s: Error while closing the file descriptor!", argv[0]);
    
    // unmap shared memory
    if(munmap(myshm, sizeof(*myshm)) == -1)
        fprintf(stderr,"%s: Error while unmapping the shared memory object!", argv[0]);
    

    return EXIT_SUCCESS;
}