/**
 * @file generator.c
 * @author Manuel Hochreiter e0671428@student.tuwien.ac.at
 * @brief Main program module.
 * 
 * This program reads the input edges and returns a arc feedback set.
 * 
 * @version 0.1
 * @date 2021-11-10
 * 
 * @copyright Copyright (c) 2021
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include "header.h"

static char *program_name = "generator";

//The exclusive_sem makes sure only one generator process is writing to the buffer at one time.
//Free and used_sem count the free and used space in the buffer.
//myshm marks the shared memory space.
static sem_t *exclusive_sem;
static sem_t *free_sem;
static sem_t *used_sem;
static struct myshm *myshm;

//this function wites lines to the buffer as described in the lecture slides.
static void buffer_write(char *value)
{
    if (sem_wait(exclusive_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_wait failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem_wait(free_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_wait failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    strncpy(myshm->solutions[myshm->wr_pos], value, 8 * 4);
    if (sem_post(used_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_post failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    myshm->wr_pos += 1;
    myshm->wr_pos %= MAX_BUFFER_LENGTH;
    if (sem_post(exclusive_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_post failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void usage(void)
{
    fprintf(stderr, "Usage: ./%s edges...\n", program_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{

    time_t t;
    srand((unsigned)time(&t));

    int c;
    int countEdges = argc - optind;
    int edges[countEdges][3];

    while ((c = getopt(argc, argv, "")) != -1)
    {
        switch (c)
        {
        case '?':
            usage();
            break;
        default:
            usage();
            break;
        }
    }

    for (int i = 0; i < countEdges; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            edges[i][j] = argv[i + optind][j] - '0';
        }
    }

    //we iterate through all vertices to find out the maximum
    int maxvertices = edges[0][0];
    int minvertices = edges[0][0];
    for (int i = 0; i < countEdges; i++)
    {
        for (int m = 0; m < 3; m = m + 2)
        {
            if (edges[i][m] > maxvertices)
            {
                maxvertices = edges[i][m];
            }
            if (edges[i][m] < minvertices)
            {
                minvertices = edges[i][m];
            }
        }
    }
    int size = maxvertices - minvertices + 1;

    //we create a list of the available vertices
    int vertices[size +3];
    for (int k = 0; k < size; k++)
    {
        vertices[k] = k + minvertices;
    }

    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        fprintf(stderr, "[%s] ERROR: shm_open failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (myshm == MAP_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: mmap failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    exclusive_sem = sem_open(SEM_2, 0);
    if (exclusive_sem == SEM_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    free_sem = sem_open(SEM_3, 0);
    if (free_sem == SEM_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    used_sem = sem_open(SEM_4, 0);
    if (used_sem == SEM_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (myshm->terminate == 0)
    {

        //now we shuffle those vertices according to the Fisher-Yates-Shuffle
        for (int i = size - 1; i > 1; i--)
        {
            int random = (rand()) % (i + 1);
            int temp = vertices[random];
            vertices[random] = vertices[i];
            vertices[i] = temp;
        }


        //inverse order function
        int orderVector[size+3];
        for (int k = 0; k < size; k++)
        {
            orderVector[vertices[k]] = k;
            
        }


        //fill algorithm result
        char *arcSet = malloc(1);

        if (arcSet == NULL)
        {
            fprintf(stderr, "[%s] ERROR: malloc failed: %s\n", program_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        int freeSpace = 0;

        for (int i = 0; i < countEdges; i++)
        {
            if (orderVector[edges[i][0]] > orderVector[edges[i][2]])
            {
                char c1 = edges[i][0] + '0';
                char c2 = edges[i][2] + '0';
                arcSet = realloc(arcSet, freeSpace + 4);

                if (arcSet == NULL)
                {
                    fprintf(stderr, "[%s] ERROR: realloc failed: %s\n", program_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                arcSet[freeSpace + 0] = c1;
                arcSet[freeSpace + 1] = '-';
                arcSet[freeSpace + 2] = c2;
                arcSet[freeSpace + 3] = ' ';

                freeSpace = freeSpace + 4;
            }
        }
        arcSet[freeSpace - 1] = '\0';

        buffer_write(arcSet);
        free(arcSet);
    }
    if (sem_post(free_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_wait failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (munmap(myshm, sizeof(*myshm)) == -1)
    {
        fprintf(stderr, "[%s] ERROR: munmap failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(shmfd) == -1)
    {
        fprintf(stderr, "[%s] ERROR: close failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sem_close(exclusive_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sem_close(free_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sem_close(used_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}