#include <stdio.h> 
#include <sys/mman.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include "supervisor.h"
#include <fcntl.h> 
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>


/** 
 * @author Florian Jäger (11810847)
 * @brief listens to generators and prints removed edges of cyclic graph
 * @date 2021-11-13
 */

struct shm {
    unsigned int state;
    struct edge results[BUFFER_SIZE][MAX_SOLUTION];
};

volatile sig_atomic_t quit = 0;
/**
    @brief signal handler, which sets stopping
*/
void handle_signal(int signal) { printf("\nStopping started:\n"); quit = 1;}

int main() {
    //open shared memory
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        perror("shm_open error");
        return EXIT_FAILURE;
        }

    if (ftruncate(shmfd, sizeof(struct shm)) < 0) {
        perror("ftruncate error");
        return EXIT_FAILURE;
    }
    //map shared memory
    struct shm *shm;
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    shm->state = 0;
    if (shm == MAP_FAILED) {
        perror("mmap error");
        return EXIT_FAILURE;
    }
    if (close(shmfd) == -1) {
        perror("close error");
        return EXIT_FAILURE;
    }

    sem_unlink(SEM_1);
    sem_unlink(SEM_2);
    sem_unlink(SEM_3);
    //create semaphores
    sem_t *s1 = sem_open(SEM_1, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    sem_t *s2 = sem_open(SEM_2, O_CREAT | O_EXCL, 0600, 0);
    sem_t *s3 = sem_open(SEM_3, O_CREAT | O_EXCL, 0600, 0);
    sem_post(s3);

    int best = 100;
    int rd_pos = 0;
    int setCount;
    //signal handling
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    while (!quit) {
        if(sem_wait(s2) == -1) {
                if (errno == EINTR) continue;
                break;
            }
        setCount = 0;
        
        if(shm->results[rd_pos][0].v1 == 0 && shm->results[rd_pos][0].v2 == 0) {
            printf("The graph is acylic!\n");
            shm->state = 1;
            break;
        }
        for (int i = 0; i < MAX_SOLUTION; i++) {
            if(shm->results[rd_pos][i].v1 > 0 || shm->results[rd_pos][i].v2 > 0) setCount++;
        }

        if(setCount < best) {
            best = setCount;
            printf("\nSolution for %d edges: ", setCount);
            for (int i = 0; i < setCount; i++) {
                if(!(shm->results[rd_pos][i].v1 == 0 && shm->results[rd_pos][i].v2 == 0)) {
                    int v1 = shm->results[rd_pos][i].v1;
                    int v2 = shm->results[rd_pos][i].v2;
                    printf(" %d-%d ",v1,v2);
                }
            }
            printf("\n");
        }

        rd_pos += 1;
        rd_pos %= BUFFER_SIZE;
        //sleep(1);
        sem_post(s1);

    }
    shm->state = 1;
    
    //unmap shared memory
    if (munmap(shm, sizeof(*shm)) == -1) {
        perror("error unmap shared memory");
        return EXIT_FAILURE;
    }

    //unlink shared memory
    if (shm_unlink(SHM_NAME) == -1) {
        perror("error unlink shared memory");
        return EXIT_FAILURE;
    }
    //close semaphores
    sem_close(s1);
    sem_close(s2);
    sem_close(s3);
    sem_unlink(SEM_1);
    sem_unlink(SEM_2);
    sem_unlink(SEM_3);


    return EXIT_SUCCESS;


}