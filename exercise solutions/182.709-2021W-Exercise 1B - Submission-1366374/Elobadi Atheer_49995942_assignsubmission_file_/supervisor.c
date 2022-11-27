/**
 * @file: supervisor.c
 * @author: Atheer ELobadi
 * @date: 08.11.2021
 * @brief: Find a minimal Feedback Arc Set
 * @details: This supervisor program reads the generated Feedback Arc Set of a given Graph from a memory share (circular buffer)
 *           and returns the minimal of them. The generators are terminated when the supervisor determines the graph is acyclic.
 */

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

volatile sig_atomic_t quit = 0;

#define USAGE "Usage: %s\n"



void handle_signal(int signal)
{
    fprintf(stderr, "Exiting safely..\n%s\n", strerror(errno));
    quit = 1;
}

void usage(char *program_name)
{
    printf(USAGE, program_name);
}

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    system("rm -rf /dev/shm/*01049225*");
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        printf("Error: can´t open shared memory.\n");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmfd, sizeof(struct shm)) < 0)
    {
        fprintf(stderr, "Error allocating memory:.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct shm *shm;
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm == MAP_FAILED)
    {
        fprintf(stderr, "Error: mapping file to memory.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    sem_t *sem_used;
    if ((sem_used = sem_open(SEM_CIR_BUF_R, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED)
    {
        fprintf(stderr, "%s: Error: can't open semaphore.\n%s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    sem_t *sem_free;
    if ((sem_free = sem_open(SEM_CIR_BUF_W, O_CREAT | O_EXCL, 0600, CIR_BUF_SIZE)) == SEM_FAILED)
    {
        fprintf(stderr, "%s: Error: can't open semaphore.\n%s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    sem_t *sem_mutex;
    if ((sem_mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1)) == SEM_FAILED)
    {
        fprintf(stderr, "%s: Error: can't open semaphore.\n%s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sigaction sa = {.sa_handler = handle_signal};
    sigaction(SIGINT, &sa, NULL);

    shm->writeIndex = 0; 
    int read_index = 0;
    
    int min_size = MAX_SET_SIZE;
    while (!quit)
    {
        if (sem_wait(sem_used) == -1)
        {
            if (errno == EINTR)
                continue;
            exit(EXIT_FAILURE);
        }

        int size = shm->data[read_index].size;
        if ( size < min_size && size > 0)
        {
            min_size = size;
            printf("[%s]: Solution of size \033[0;33m%d\033[0;0m: ", argv[0], size);
            for (int i = 0; i < size; i++)
            {
                printf("\033[0;36m%d-%d\033[0;0m ", shm->data[read_index].edge[i].u, shm->data[read_index].edge[i].v);
            }
            printf("\n");
        }
        if (size == 0)
        {
            quit = 1;
            printf("\033[0;33mThe graph is asyclic\033[0;0m\n");
            continue; 
        }
        read_index = (read_index + 1) % CIR_BUF_SIZE;
        if (sem_post(sem_free) == -1)
        {
            if (errno == EINTR)
                continue;
            exit(EXIT_FAILURE);
        };
    }
    shm->terminated = 1;
    sleep(1);
    printf("done!\n");

    // unmap shared memory:
    if (munmap(shm, sizeof(*shm)) == -1)
    {
        fprintf(stderr, "Error: Can´t unmap shared memory.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
 
    if (close(shmfd) == -1)
    {
        fprintf(stderr, "Error: Can´t close shared memory.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }    
    
    // remove shared memory object:
    if (shm_unlink(SHM_NAME) == -1)
    {
        fprintf(stderr, "Error: Can´t remove shared memory.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sem_close(sem_used) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_free) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_mutex) == -1)
    {
        exit(EXIT_FAILURE);
    }
    sem_unlink(SEM_CIR_BUF_R);
    sem_unlink(SEM_CIR_BUF_W);
    return EXIT_SUCCESS;
}