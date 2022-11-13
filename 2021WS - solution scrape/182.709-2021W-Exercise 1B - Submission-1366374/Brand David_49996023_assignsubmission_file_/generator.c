/**
* @file generator.c
* @author David Brand <e11905164@student.tuwien.ac.at>
* @date 6.11.2021
*
* @brief The generator part of the program module.
*
* @details Usage: generator EDGE1... edges are written as 'int-int'.
* The generator part of the program takes a graph as input.
* The program repeatedly generates a random solution to the 3-coloring problem by removing edges.
* It then writes the solution to the circular buffer. It does this until notified
* by the supervisor to terminate. The program accepts graphs of any size.
**/

#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include "struct.h"

/**
* @brief Main program entry point.
*
* @details The function parses the arguments given when calling the program
* as edges and saves them in an array. It then opens all the semaphores initialized
* by the supervisor and tries to open and map the shared memory.
* The function then creates a random coloring of the given graph.
* If two vertices connected by an edge have the same color, the function remembers
* this edge and increments the counter of how many edges have to be removed to make
* the graph 3-colorable. It then writes the solution to the circular buffer in the shared memory.
* The function does this procedure until the supervisor notifies it to terminate.
* If the generator gets notified, the function tries to close and unmap the shared memory
* and close and unlink all the semaphores.
*
* @param argc the argument counter.
* @param argv the argument vector.
*
* @return Returns EXIT_SUCCESS or EXIT_FAILURE on error.
**/

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        fprintf(stderr, "[%s] ERROR: the generator program requires at least one argument", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct edge edges[argc - 1];
    struct solution solutions;
    solutions.count = 0;
    unsigned int begin, end;
    unsigned int max = 0;
    unsigned int cnt = 0;
    srand(time(NULL));

    while ((argc - cnt - 1) > 0)
    {
        if (sscanf(argv[cnt + 1], "%u-%u", &begin, &end) != 2)
        {
            fprintf(stderr, "[%s] ERROR: failure whilst reading edge argument", argv[0]);
            exit(EXIT_FAILURE);
        }
        if (begin > max)
        {
            max = begin;
        }
        if (end > max)
        {
            max = end;
        }
        edges[cnt] = (struct edge) {begin, end};
        cnt++;
    }
    max++; //because vertices start at 0

    sem_t *s2 = sem_open(SEM_2, 0);
    sem_t *s1 = sem_open(SEM_1, 0);
    sem_t *s3 = sem_open(SEM_3, 0);

    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1)
    {
        fprintf(stderr, "[%s] ERROR: --generator-- shm open failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: --generator-- shm mmap failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    while ((myshm -> done) != 1)
    {
        sem_wait(s1);

        int color[max];
        int i = 0;
        while (i < max)
        {
            color[i] = rand() % 3;
            i++;
        }

        i = 0;
        solutions.count = 0;
        while (i < argc -1)
        {
            if (color[edges[i].from] == color[edges[i].to])
            {
                solutions.data[solutions.count].from = edges[i].from;
                solutions.data[solutions.count].to = edges[i].to;
                solutions.count += 1;
            } 
            i++;
        }
        
        sem_wait(s3);
        
        myshm -> data[myshm -> writeNode].count = solutions.count;
        for (int j = 0; j < solutions.count; j++)
        {
            myshm -> data[myshm -> writeNode].data[j].from = solutions.data[j].from;
            myshm -> data[myshm -> writeNode].data[j].to = solutions.data[j].to;
        }

        myshm -> writeNode = (myshm -> writeNode + 1) % MAX_DATA;

        sem_post(s2);
        sem_post(s3);
    }
    
    sem_post(s3);

    if (close(shmfd) == -1)
    {
        fprintf(stderr, "[%s] ERROR: --generator-- shm close failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (munmap(myshm, sizeof(*myshm)) == -1)
    {
        fprintf(stderr, "[%s] ERROR: --generator-- shm unmap failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    sem_close(s2);
    sem_close(s1);
    sem_close(s3);
    sem_unlink(SEM_1);
    sem_unlink(SEM_2);
    sem_unlink(SEM_3);

    exit(EXIT_SUCCESS);
}