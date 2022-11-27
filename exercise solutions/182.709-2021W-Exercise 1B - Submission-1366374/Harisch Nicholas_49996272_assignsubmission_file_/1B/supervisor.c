/**
 * @file supervisor.c
 * @author Nicholas Harisch 11705361<harischnicholas@gmail.com>
 * @date 12.11.2021
 *
 * @brief Supervisor accepting solutions from generators.
 * 
 * This program accepts 3-color solutions from generator instances through a shared memory object. Synchronization is provided
 * by using semaphores. Keeps track of the solution with the least amount of removed edges.
 **/
#include "supervisor.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

struct Ring_Buffer *ringbuffer; /**< Ringbuffer in the shared memory */
int shmfd;                      /**< File descriptor */
sem_t *sem_free;                /**< Semaphore keeping track of free space in ring buffer */
sem_t *sem_used;                /**< Semaphore keeping track of used space in ring buffer */
sem_t *sem_block;               /**< Semaphore to synch. writing to ring buffer */

/**
 * Cleanup function.
 * @brief This function cleans up shared memory handling and semaphores.
 * @details global variables: sem_free, sem_used, sem_block, shmfd, ringbuffer
 */
void cleanUp()
{
    (*ringbuffer).status = 0;

    //close file descriptor
    if (close(shmfd) == -1)
    {
        exit(EXIT_FAILURE);
    }
    //unmap memory object   
    if (munmap(ringbuffer, sizeof(struct Ring_Buffer)) == -1)
    {
        exit(EXIT_FAILURE);
    }

    //remove shared memory object
    if (shm_unlink(SHM_NAME) == -1)
    {
        exit(EXIT_FAILURE);
    }

    sem_close(sem_free);
    sem_close(sem_block);
    sem_close(sem_used);

    sem_unlink(SEM_USED);
    sem_unlink(SEM_BLOCK);
    sem_unlink(SEM_FREE);
}




/**
 * Program entry point.
 * @brief The program starts here. This function takes care of the ring buffer management and the solutions
 * for the 3-color problem coming from the generators.
 * @details Handles signal handling, creating semaphores, the shared memory ring buffer and then loops (if semaphores allow) through
 * the ring buffer at the fiven read position. If a solution suggested by a generator is smaller than the curren min. solution, it gets overwritten
 * if a proposed solution size is 0, the graph is 3-colorable and the program terminates. Otherwise its keeps waiting for new and better solutions.
 * global variables: sem_free, sem_used, sem_block, shmfd, ringbuffer
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[])
{
    //Error if any arguments given
    if (argc > 1)
    {
        printf("Too many arguments \n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, cleanUp);
    signal(SIGTERM, cleanUp);

    //Create semaphores
    sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_SIZE);
    sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    sem_block = sem_open(SEM_BLOCK, O_CREAT | O_EXCL, 0600, 1);

    //Create or open shared mem obj
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        cleanUp();
        exit(EXIT_FAILURE);
    }

    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct Ring_Buffer)) < 0)
    {
        cleanUp();
        exit(EXIT_FAILURE);
    }
    //map shared memory object
    ringbuffer = mmap(NULL, sizeof(*ringbuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (ringbuffer == MAP_FAILED)
    {
        cleanUp();
        exit(EXIT_FAILURE);
    }

    //Sets status to 1 so generators will start generating solutions, set 0 to stop
    (*ringbuffer).status = 1;
    (*ringbuffer).rd_pos = 0;
    (*ringbuffer).wr_pos = 0;
    (*ringbuffer).solutions[(*ringbuffer).rd_pos].size = SOLUTION_MAX + 60;

    struct Solution currentMinSolution; /**< holds the current smallest sized solution */
    currentMinSolution.size = SOLUTION_MAX + 1;


    //as long as status == 1, as soon as semaphore allows, the loop will check if the solutions in the ring buffer are smaller than the current min. solution
    while ((*ringbuffer).status == 1)
    {
        sem_wait(sem_used);

        fflush(stdout);
        if ((*ringbuffer).solutions[(*ringbuffer).rd_pos].size < currentMinSolution.size)
        {

            //Set current min solution to soltion from buffer at rd_pos
            currentMinSolution = (*ringbuffer).solutions[(*ringbuffer).rd_pos];

            //is size of solution == 0, the graph is 3 colorable so program terminates, otherwise it prints solution and continues
            if ((*ringbuffer).solutions[(*ringbuffer).rd_pos].size == 0)
            {
                printf("The graph is 3-colorable! \n");
                cleanUp();
                exit(EXIT_SUCCESS);
            }
            else
            {
                printf("Solution with %lu edges:", currentMinSolution.size);
                for (int i = 0; i < currentMinSolution.size; i++)
                {
                    printf(" %i-%i", currentMinSolution.edges[i].a, currentMinSolution.edges[i].b);
                    fflush(stdout);
                }
            }
            printf("\n");
        }

        //Increase buffer read pos.
        (*ringbuffer).rd_pos = (*ringbuffer).rd_pos + 1;
        (*ringbuffer).rd_pos = (*ringbuffer).rd_pos % MAX_SIZE;
        sem_post(sem_free);
    }

    cleanUp();
    exit(EXIT_SUCCESS);
}
