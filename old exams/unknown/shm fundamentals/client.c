d#include <errno.h> // for errno
#include <fcntl.h> // for open()
#include <semaphore.h> // for sem_open, and sem stuff
#include <stdio.h> // for printf()
#include <stdlib.h> // for a lot (malloc, strtol...)
#include <string.h> // for memcpy, strcat, strncat, strcpy
#include <sys/mman.h> // for shm_open and mmap
#include <sys/stat.h> // ftruncate etc.
#include <unistd.h> // ftruncate etc.
#include <stdbool.h>

#include <semaphore.h>

#define SEM_NAME_REQ "/semReqB12022502"
#define SEM_NAME_RES "/semResB12022502"

#define SHM_NAME "/myshmB12022502"
#define SHM_SIZE 8192

#define PROGRAM_NAME "client.c"

void error_exit(char *message);

int main(int argc,char**argv){
    int shmfd = shm_open(SHM_NAME,O_RDWR,0600);
    if(shmfd == -1) error_exit("shmfd");

    char* shmp = mmap(NULL,SHM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmfd,0);
    if(shmp == MAP_FAILED) error_exit("shmp");

    if(close(shmfd) == -1) error_exit("close shmfd");

    sem_t *sem_req = sem_open(SEM_NAME_REQ, 0);
    sem_t *sem_res = sem_open(SEM_NAME_RES, 0);
    if(sem_req == SEM_FAILED) error_exit("sem req");
    if(sem_res == SEM_FAILED) error_exit("sem res");

    // act, as if both got executed at the same time (cause the ... part is not even necessary!)
    // precondition: server got executed before client. (TODO: can this be avoided)
    strcpy(shmp,"Hello World!");
    sem_post(sem_req);
    if(sem_wait(sem_res) == -1) error_exit("wait");
    printf("server responded to request, which means shared memory can now be unmapped & semaphores be closed\n");

    if (munmap(shmp,SHM_SIZE) == -1) error_exit("munmap");
    if (sem_close(sem_req) == -1) error_exit("sem_close req");
    if (sem_close(sem_res) == -1) error_exit("sem_close res");

    return EXIT_SUCCESS;
}

void error_exit(char *message){
    fprintf(stderr,"%s: %s\n",PROGRAM_NAME, message);
    fprintf(stderr,"Error Code: %d\n",errno);
    exit(EXIT_FAILURE);
}
