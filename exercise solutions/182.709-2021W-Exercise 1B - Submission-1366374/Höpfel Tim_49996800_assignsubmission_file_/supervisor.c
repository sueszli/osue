/**
 * @file supervisor.c
 * @author Tim Höpfel Matr.Nr.: 01207099 <e01207099@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief supervisor-process collecting results from generator
 *
 * The supervisor sets up the shared memory and the semaphores and initializes the circular buffer required
 * for the communication with the generators. It then waits for the generators to write solutions to the
 * circular buffer.
 * The supervisor program takes no arguments.
 * It terminates, when a arc-result-set with the size of 0 is collected, or until it receives a SIGINT or a SIGTERM signal.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#define SHM_NAME "/01207099_arcset"
#define MAX_SOLUTION (8)
#define CIRC_BUFFER_SIZE (1024)
#define SEM_FREE "/01207099_sem_free"
#define SEM_USED "/01207099_sem_used"
#define SEM_WRITE "/01207099_sem_write"

struct edge
{
    long start;
    long end;
};
struct arcset
{
    int size;
    struct edge solution[MAX_SOLUTION];
};
struct arcshm {
    int state;
    unsigned int wr_pos;
    struct arcset arcsets[CIRC_BUFFER_SIZE];
};
volatile sig_atomic_t quit = 0;
void handle_signal(int signal) {
    quit = 1;
}

/**
 * Program entry point.
 * @brief The program starts here.
 * @details The supervisor sets up the shared memory and the semaphores and initializes the circular buffer required
 * for the communication with the generators. It then waits for the generators to write solutions to the
 * circular buffer.
 * The supervisor program takes no arguments.
 * It terminates, when a arc-result-set with the size of 0 is collected, or until it receives a SIGINT or a SIGTERM signal.
 * @return Returns EXIT_SUCCESS.
 */
int main (int argc, char *argv[]) {
    //signalhandling for SIGINT and SIGTERM
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

//set up shared memory
    //create shared memory object
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "shm_open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct arcshm)) < 0) {
        fprintf(stderr, "ftruncate failed: %s\n", strerror(errno));
        close(shmfd);
        exit(EXIT_FAILURE);
    }
    // map shared memory object:
    struct arcshm *arcshm;
    arcshm = mmap(NULL, sizeof(*arcshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0); //0 offset
    if (arcshm == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        close(shmfd);
        exit(EXIT_FAILURE);
    }
    if (close(shmfd) == -1) {
        arcshm->state = (-1);
        munmap(arcshm, sizeof(*arcshm));
        fprintf(stderr, "close failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //set semaphores
    sem_t *s_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, CIRC_BUFFER_SIZE); //The whole buffer is free to write solutions into it
    sem_t *s_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0); //nothing is used in the beginning, nothing can be read
    sem_t *s_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1); //It is allowed, for one Thread to write at position 0


//initialize circular buffer
    arcshm->state = 1; //generators may generate
    arcshm->wr_pos = 0; //starting position

    //create arcset to remember best solution
    struct arcset best_solution;
    best_solution.size = MAX_SOLUTION + 1;
    struct arcset new_solution;

    int rd_pos = 0;
    while(!quit) {
//waits for generators to write solutions to the circular buffer
        if(sem_wait(s_used) == (-1)) {
            if(errno == EINTR) {
                break;
            } else {
                fprintf(stderr, "sem_wait failed: %s\n", strerror(errno));
                break;
            }
        }
//read solutions, remember best solution, output if solution gets better, terminate if solution with 0 edges is read or SIGINT / SIGTERM is beeing received
        new_solution = arcshm->arcsets[rd_pos];
        if(new_solution.size < best_solution.size) {
            best_solution.size = new_solution.size;
            memcpy(best_solution.solution, new_solution.solution, sizeof(best_solution.solution));
            if(best_solution.size > 0) {
                printf("Solution with %i edges:", best_solution.size);
                for(int i = 0; i < best_solution.size; i++) {
                    printf(" %li-%li", best_solution.solution[i].start, best_solution.solution[i].end);
                }
                printf("\n");
            } else {
                printf("The graph is acyclic!\n");
                break;
            }
        }
        sem_post(s_free);
        rd_pos += 1;
        rd_pos %= CIRC_BUFFER_SIZE;

    }
//termination
    //notifie all generators, that they should terminate as well, by setting a variable in shared memory
    arcshm->state = (-1);
//unlink all shared resources
    // unmap shared memory:
    if (munmap(arcshm, sizeof(*arcshm)) == -1) {
        fprintf(stderr, "munmap failed: %s\n", strerror(errno));
    }
    if (shm_unlink(SHM_NAME) == -1) {
        fprintf(stderr, "shm_unlink of SHM_NAME failed: %s\n", strerror(errno));
    }
    // remove shared memory object:

    if(sem_close(s_free) == -1) {
        fprintf(stderr, "sem_close of s_free failed: %s\n", strerror(errno));
    }
    if(sem_close(s_used) == -1) {
        fprintf(stderr, "sem_close of s_used failed: %s\n", strerror(errno));
    }
    if(sem_close(s_write) == -1) {
        fprintf(stderr, "sem_close of s_write failed: %s\n", strerror(errno));
    }

    if(sem_unlink(SEM_FREE) == -1) {
        fprintf(stderr, "sem_unlink of SEM_FREE failed: %s\n", strerror(errno));
    }
    if(sem_unlink(SEM_USED) == -1) {
        fprintf(stderr, "sem_unlink of SEM_USED failed: %s\n", strerror(errno));
    }
    if(sem_unlink(SEM_WRITE) == -1) {
        fprintf(stderr, "sem_unlink of SEM_WRITE failed: %s\n", strerror(errno));
    }

    //exit
    return EXIT_SUCCESS;
}
