#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>

#define SHM_NAME "/myshm12022502circbufA"
#define SHM_SIZE 8

#define SEM_NAME_USED   "/sem12022502usedA"
#define SEM_NAME_FREE   "/sem12022502freeA"
#define SEM_NAME_CLIENT "/sem12022502clientA"

#define PROGRAM_NAME "client.c"

void error_exit(char *message);


int main(int argc,char**argv){
    int shmfd = shm_open(SHM_NAME,O_RDWR,0600);
    if(shmfd == -1) error_exit("shmfd");

    char *shmp = mmap(NULL,SHM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmfd,0);
    if(shmp == MAP_FAILED) error_exit("shmp");

    if(close(shmfd) == -1) error_exit("close shmfd");

    sem_t *sem_used = sem_open(SEM_NAME_USED, 0); // 0 ...
    sem_t *sem_free = sem_open(SEM_NAME_FREE, 0);
    sem_t *sem_client = sem_open(SEM_NAME_CLIENT, 0);
    if(sem_used == SEM_FAILED) error_exit("sem setup");
    if(sem_free == SEM_FAILED) error_exit("sem server");
    if(sem_client == SEM_FAILED) error_exit("sem client");

    char *str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    int write_pos = 0; // all clients need to have the same value for this... how?

    int i = 0;
    while(str[i] != '\0'){
        // sem_wait(sem_client);
        char c = str[i];
        i++;
        if (isupper(c)){
            if (sem_wait(sem_free) == -1) error_exit("sem_wait free");
            char c1 = c + 6;
            if (c1 > 'Z') c1 -= 26;
            printf("write_pos: %d\n",write_pos);
            shmp[write_pos] = c1;
            write_pos++;
            write_pos %= SHM_SIZE;
            sem_post(sem_used);
        }
        // sem_post(sem_client);
    }



    if (munmap(shmp,SHM_SIZE) == -1) error_exit("munmap");
    if (sem_close(sem_used) == -1) error_exit("sem_close setup");
    if (sem_close(sem_free) == -1) error_exit("sem_close server");
    if (sem_close(sem_client) == -1) error_exit("sem_close client");

    return EXIT_SUCCESS;
}

void error_exit(char *message){
    fprintf(stderr,"%s: %s failed\n",PROGRAM_NAME, message);
    fprintf(stderr,"Error Code: %d\n",errno);
    exit(EXIT_FAILURE);
}
