/**
* @file supervisor.c
* @author David Brand <e11905164@student.tuwien.ac.at>
* @date 6.11.2021
*
* @brief The supervisor part of the program module.
*
* @details Usage: supervisor
* The supervisor part of the program sets up the shared memory and semaphores.
* The supervisor program takes no arguments.
* It waits for the generators to write solutions to the circular buffer.
* The program then reads from the circular buffer and remembers the solution
* with the least edges to remove and writes it to stdout. 
* If a solution with 0 edges is found it terminates and notifies the generators
* to terminate aswell. It then closes and unlinks all shared recources.
* If the signal SIGINT or SIGTERM is sent, the program terminates aswell.
**/

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <limits.h>
#include "struct.h"

volatile sig_atomic_t quit = 0;

void handle_signal(int signal)
{
    quit = 1;
}

/**
* @brief Main program entry point.
*
* @details The function uses signal handling to terminate if SIGTERM or SIGINT is sent.
* It initializes the semaphores and tries to initialize and map the shared memory.
* The function then reads the solutions from the circular buffer and if it is 
* better than the current best solution, it writes them to stdout.
* If a solution with 0 edges is found it terminates.
* After such a solution is found or SIGTERM or SIGINT is sent, it then tries to
* close, unmap and unlink the shared memory and close und unlink all the semaphores.
*
* @param argc the argument counter.
* @param argv the argument vector.
*
* @return Returns EXIT_SUCCESS or EXIT_FAILURE on error.
**/

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        fprintf(stderr, "[%s] ERROR: the supervisor program takes no arguments", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sem_t *s1 = sem_open(SEM_1, O_CREAT | O_EXCL, 0600, MAX_DATA);
    sem_t *s2 = sem_open(SEM_2, O_CREAT | O_EXCL, 0600, 0);
    sem_t *s3 = sem_open(SEM_3, O_CREAT | O_EXCL, 0600, 1);

    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shmfd == -1)
    {
        fprintf(stderr, "[%s] ERROR: --supervisor-- shm open failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmfd, sizeof(struct myshm)) < 0)
    {
        fprintf(stderr, "[%s] ERROR: --supervisor-- ftruncate failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: --supervisor-- shm mmap failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    myshm -> writeNode = 0;
    myshm -> readNode = 0;
    myshm -> done = 0;
    unsigned int minCurrentEdges = MAX_DATA;

    while (!quit)
    {
        sem_wait(s2);

        unsigned int currentEdges = myshm -> data[myshm -> readNode].count;
        if (minCurrentEdges > currentEdges)
        {
            minCurrentEdges = currentEdges;

            if (minCurrentEdges == 0 && !quit)
            {
            printf("The given graph is 3-colorable without removing any edges.\n");
            break;
            } else if (!quit)
            {
                fprintf(stdout, "Solution with %u edges:", minCurrentEdges);
            }

            for (int i = 0; i < minCurrentEdges; i++)
            {
                unsigned int from = myshm -> data[myshm -> readNode].data[i].from;
                unsigned int to = myshm -> data[myshm -> readNode].data[i].to;
                fprintf(stdout, " %u-%u", from, to);
            }
            printf("\n");
        }
    
        myshm -> readNode = (myshm -> readNode + 1) % MAX_DATA;

        sem_post(s1);
    }

    myshm -> done = 1;

    if (close(shmfd) == -1)
    {
        fprintf(stderr, "[%s] ERROR: --supervisor-- shm close failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (munmap(myshm, sizeof(*myshm)) == -1)
    {
        fprintf(stderr, "[%s] ERROR: --supervisor-- shm unmap failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(SHM_NAME) == -1)
    {
        fprintf(stderr, "[%s] ERROR: --supervisor-- shm unlink failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_DATA; i++)
    {
        sem_post(s1);
    }

    sem_close(s3);
    sem_close(s2);
    sem_close(s1);
    sem_unlink(SEM_1);
    sem_unlink(SEM_2);
    sem_unlink(SEM_3);
    exit(EXIT_SUCCESS);
}