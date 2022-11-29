#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include <semaphore.h>

#define SEM_NAME_REQ "/semReqB12022502"
#define SEM_NAME_RES "/semResB12022502"

#define SHM_NAME "/myshmB12022502"
#define SHM_SIZE 8192

#define PROGRAM_NAME "server.c"

void error_exit(char *message);

int main(int argc,char**argv){
    int shmfd = shm_open(SHM_NAME,O_RDWR|O_CREAT,0600);
    if(shmfd == -1) error_exit("shmfd");

    if(ftruncate(shmfd,SHM_SIZE) == -1) error_exit("ftruncate");

    char* shmp = mmap(NULL,SHM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmfd,0);
    if(shmp == MAP_FAILED) error_exit("shmp");

    if(close(shmfd) == -1) error_exit("close shmfd");

    sem_unlink(SEM_NAME_REQ);
    sem_unlink(SEM_NAME_RES);
    sem_t *sem_req = sem_open(SEM_NAME_REQ, O_CREAT|O_EXCL,0600,0);
    sem_t *sem_res = sem_open(SEM_NAME_RES, O_CREAT|O_EXCL,0600,0);
    if(sem_req == SEM_FAILED) error_exit("sem req");
    if(sem_res == SEM_FAILED) error_exit("sem res");


    char message[SHM_SIZE];
    // act, as if both got executed at the same time, but server has to write first
    if (sem_wait(sem_req) == -1) error_exit("wait");
    strcpy(message,shmp);
    sem_post(sem_res);
    printf("%s\n",message);

    // TODO: include error_exit in header for both, change makefile even necessary?
    // TODO: test that thing with sem_unlink real quick
    if (munmap(shmp,SHM_SIZE) == -1) error_exit("munmap");
    if (shm_unlink(SHM_NAME) == -1) error_exit("shm_unlink");
    if (sem_close(sem_req) == -1) error_exit("sem_close req");
    if (sem_close(sem_res) == -1) error_exit("sem_close res");
    if (sem_unlink(SEM_NAME_REQ) == -1) error_exit("sem_unlink req");
    if (sem_unlink(SEM_NAME_RES) == -1) error_exit("sem_unlink res");

    return EXIT_SUCCESS;
}

void error_exit(char *message){
    fprintf(stderr,"%s: %s\n",PROGRAM_NAME, message);
    fprintf(stderr,"Error Code: %d\n",errno);
    exit(EXIT_FAILURE);
}
