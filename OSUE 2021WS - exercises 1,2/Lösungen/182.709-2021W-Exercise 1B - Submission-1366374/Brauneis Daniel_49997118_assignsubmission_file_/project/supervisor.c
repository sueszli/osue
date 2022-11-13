/**
*@file supervisor.c
*@author daniel brauneis 12021357
*@details supervises generators and picks the best solution provided by a generator via a circular buffer in shared memory
*@date 2021-11-10
*Main Program
*/
#include <stdio.h>
#include <fcntl.h> 
#include <stdio.h> 
#include <sys/mman.h>
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#define SHM_NAME "/e12021357myshm"
#define SEM_FREE "/e12021357sem_free"
#define SEM_USED "/e12021357sem_used"

typedef struct {int a,b;} edge;
typedef struct {edge edges[sizeof(edge)*8]; int size;} arcset;
struct arcset {
    edge edges[sizeof(edge)*8];
    int size;
};

void handle_signal(int signal);

volatile sig_atomic_t quit = 0;

/**
* Main Function
* @brief This is the main function of the program
* @param arc number of command-line arguments
* @param argv list of arguments
*/
int main(int argc, char **argv){
    printf("supervisor startet\n");
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_FREE);
    sem_unlink(SEM_USED);
    
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
    if (shmfd == -1){
        fprintf(stderr, "Error while opening shared memory, Errno: %i \n", errno);
       printf("Soemthing went wrong: %s\n", strerror(errno));
	     exit(EXIT_FAILURE);
    }
    if (ftruncate(shmfd, sizeof(arcset)*3) < 0){
        fprintf(stderr, "Error while setting size of shared memory");
        exit(EXIT_FAILURE);
    }
    arcset *e12021357myshm;
    
    e12021357myshm = mmap(NULL, sizeof(e12021357myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (e12021357myshm == MAP_FAILED) exit(EXIT_FAILURE);
    if (close(shmfd) == -1) exit(EXIT_FAILURE);
    
    //sem_open
    sem_t *e12021357free =  sem_open(SEM_FREE, O_CREAT | O_EXCL, 0777, 3);
    sem_t *e12021357used =  sem_open(SEM_USED, O_CREAT | O_EXCL, 0777, 0);
    arcset current = {{{0}}, 9};
    int rd_pos = 0;
    while(!quit){
        sem_wait(e12021357used);
        if(e12021357myshm[rd_pos].size == 0 && !quit){
            fprintf(stderr, "The graph is acyclic\n");
            e12021357myshm[rd_pos].size = 999999999;
            quit = 1;
        }
        if(current.size > e12021357myshm[rd_pos].size && !quit){
            current = e12021357myshm[rd_pos];            
            printf("Solutions with %d edges: ", current.size);
            for(int i = 0; i < current.size;i++){
                //workaround for 0 values
                int a = current.edges[i].a;
                int b = current.edges[i].b;
                if(a == 999999999) a = 0;
                if(b == 999999999) b = 0;
                printf("%d-%d " ,a, b);
            } 
            printf("\n");
        }
        sem_post(e12021357free);
        rd_pos += 1;
        rd_pos %= sizeof(e12021357myshm)-1;        
    }
    e12021357myshm[rd_pos].size = 999999999;
    printf("Telling generators to quit on pos: %d", rd_pos);
    //Code
    if(munmap(e12021357myshm, sizeof(*e12021357myshm)) == -1) exit(EXIT_FAILURE);
    shm_unlink(SHM_NAME);
    //semclose
    sem_close(e12021357free);
    sem_close(e12021357used);
    //sem_unlink
    sem_unlink(SEM_FREE);
    sem_unlink(SEM_USED);
}

void handle_signal(int signal) {
    quit = 1;
}
