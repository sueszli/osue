/**
 * @file   supervisor.c
 * @author Nadejda Capova (11923550)
 * @date   26.10.2021
 *
 * @brief Algorithm which makes a graph 3-colorable by removing the least edge possible
 *
 * @details Multiple processes generate the random sets of
 * edges in parallel and report their results to a supervisor process,
 * which remembers the set with the least edges.
 * The supervisor sets up the shared memory and the semaphores and initializes the circular buffer required
 * for the communication with the generators. It then waits for the generators to write solutions to the
 * circular buffer.
 * The supervisor program takes no arguments.
 *
 **/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utility.h"



/*name of the program*/
static char *myprog;
//signal handling
volatile sig_atomic_t quit = 0; //Set global quit flag
void handle_signal(int signal) { quit = 1; }
/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variable: myprog
 */
static void usage(void) {
    fprintf(stderr, "Usage: %s  \n", myprog);
    exit(EXIT_FAILURE);
}


/**
 * @brief The entry point of the program.
 * @details This is the only function that executes the whole program. Every time a better solution
 * than the previous best solution is found, the supervisor writes the new solution to standard output.
 * A solution with 0 edges terminates the program.
 * @param argc  - argument counter
 * @param argv - argument values stored in an array
 *
 * @return Returns 0 upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[]) {
//The supervisor program takes no arguments.
    myprog = argv[0];
    if (argc - optind != 0) {
        usage();
    }
    //file descriptor of the open shared memory
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        fprintf(stderr, "%s ERROR: Cannot open shared memmory: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // set the size of the shared memory
    if (ftruncate(shmfd, sizeof(struct myshm)) < 0)
    {
        fprintf(stderr, "[%s] ftruncate failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }

    //processes communicate with each other
    // by means of a circular buffer, which is implemented using shared semaphores and a shared memory
    // map shared memory object
    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE,
                 MAP_SHARED, shmfd, 0);

   
    myshm->ifStop = 1; //Before terminating, the supervisor notifies all generators that they should terminate as well
    if (myshm == MAP_FAILED)
    {
        fprintf(stderr, "[%s] mmap failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }

    //remove all semaphores
    sem_unlink(FILL);
    sem_unlink(EMPTY);
    sem_unlink(MUTEX);

    // creates and/or open the shared memory object
    sem_t *sem_fill = sem_open(FILL, O_CREAT | O_EXCL, 0600, 0);
    sem_t *sem_empty = sem_open(EMPTY, O_CREAT | O_EXCL, 0600, sizeof(*myshm));
    sem_t *sem_mutex = sem_open(MUTEX, O_CREAT | O_EXCL, 0600, 1);

    if ((sem_fill == SEM_FAILED) || (sem_empty == SEM_FAILED) || (sem_mutex == SEM_FAILED))
    {
        fprintf(stderr, "sem_open failed: %s  in %s\n", strerror(errno), argv[0]);
        exit(EXIT_FAILURE);
    }
    //signal configuration
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);

    arrayEdge_t best; //best solution
    best.countEdges = 1000;
    int rd_pos; //reading osition in shared memory (circular buffer)

    while (!quit){
        if(sem_wait(sem_fill)==-1){
            if(errno==EINTR){
                continue;
            }
            exit(1);
            }
            arrayEdge_t val = myshm->result[rd_pos];
            //read from circular buffer
            rd_pos+=1;
            rd_pos%=MAX_DATA;
            sem_post(sem_empty);
            //find the best solution with less removed edges
            if(val.countEdges<best.countEdges){
                best=val;
                if(best.countEdges==0){
                    printf("[%s] The graph is 3-colorable!\n",myprog); //a solution with 0 edges
                    fflush(stdout);
                    break;
                }
                 printf("[%s] Solution with %d edges: ", myprog, val.countEdges); //a solution with n edges
                 for(size_t i =0; i<val.countEdges; i++){
                     printf("%d-%d ", val.node[i].a, val.node[i].b);
                 }
                 printf("\n");
                fflush(stdout);
            }
    }

    myshm->ifStop = 0; //notifies all generators to stop when found result 0

     // unmap shared memory //TODO
    if (munmap(myshm, sizeof *(myshm)) == -1)
    {
        fprintf(stderr, "[%s] munmap failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (close(shmfd) == -1)
    {
        fprintf(stderr, "[%s] close failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // remove shared memory object:
    if (shm_unlink(SHM_NAME) == -1)
    {
        fprintf(stderr, "[%s] shm_unlink failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }
    //close semaphores  //TODO: HEADER
    sem_close(sem_fill);
    sem_close(sem_empty);
    sem_close(sem_mutex);

    //remove all semaphores
    sem_unlink(FILL);
    sem_unlink(EMPTY);
    sem_unlink(MUTEX);

    return 0;
}
