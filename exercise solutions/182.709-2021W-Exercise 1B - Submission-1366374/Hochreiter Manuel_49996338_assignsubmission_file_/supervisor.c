/**
 * @file supervisor.c
 * @author Manuel Hochreiter e0671428@student.tuwien.ac.at
 * @brief Main program module.
 * 
 * This program initializes the shared memory space and waits for the generators to deliver results.
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

static char *program_name = "supervisor";

//The exclusive_sem makes sure only one generator process is writing to the buffer at one time.
//Free and used_sem count the free and used space in the buffer.
//myshm marks the shared memory space.
static sem_t *exclusive_sem;
static sem_t *free_sem;
static sem_t *used_sem;
static struct myshm *myshm;

volatile sig_atomic_t quit = 0;

void handle_signal(int signal)
{
    quit = 1;
}

/**
 * @brief This function reads lines from the buffer like described in the lecture slides.
 * 
 * @return a string which contains a possible solution
 **/
static char *buffer_read(void)
{
    if (sem_wait(used_sem) == -1)
    {
        if (errno == EINTR)
        {
            return NULL;
        }
        fprintf(stderr, "[%s] ERROR: sem_wait failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    char *value = malloc(8 * 4 * sizeof(char));
    strncpy(value, myshm->solutions[myshm->rd_pos], 8 * 4);
    if (sem_post(free_sem) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_wait failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    myshm->rd_pos += 1;
    myshm->rd_pos %= MAX_BUFFER_LENGTH;
    return value;
}

/**
 * @brief This function provides information about calling it correctly to stderr
 * @details global variables: program_name
 */
static void usage(void)
{
    fprintf(stderr, "Usage: ./%s \n", program_name);
    exit(EXIT_FAILURE);
}


/**
 * @brief The main function is the program entry point.
 * @details global variables: 
 * c: provides the return value of getopt
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int c;
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

    if (argc - optind > 0)
    {
        usage();
    }

    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        fprintf(stderr, "[%s] ERROR: shm_open failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmfd, sizeof(struct myshm)) < 0)
    {
        fprintf(stderr, "[%s] ERROR: ftruncate failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (myshm == MAP_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: mmap failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    exclusive_sem = sem_open(SEM_2, O_CREAT, 0600, 1);
    if (exclusive_sem == SEM_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    free_sem = sem_open(SEM_3, O_CREAT, 0600, MAX_BUFFER_LENGTH);
    if (free_sem == SEM_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    used_sem = sem_open(SEM_4, O_CREAT, 0600, 0);
    if (used_sem == SEM_FAILED)
    {
        fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    myshm->rd_pos = 0;
    myshm->wr_pos = 0;
    myshm->terminate = 0;

    int current_length = 8;
    char *current_solution;

    while (quit == 0)
    {
        char *temp = buffer_read();
        if (temp == NULL)
        {
            break;
        }
        int testSize = 0;
        int iterator = 0;

        //here we check how many edges the provided solution has
        while (temp[iterator] != '\0')
        {
            if (temp[iterator] == '-')
            {
                testSize++;
            }
            iterator++;
        }

        //we check if the new solution is better than the best one yet
        if (testSize < current_length)
        {
            current_solution = temp;
            current_length = testSize;
            if (current_length == 0)
            {
                printf("The graph is acyclic! \n");
            }
            else
            {
                printf("Solution with %d edge(s): %s \n", current_length, current_solution);
            }
        }
        free(temp);
        fflush(stdout);
        //if the graph is acyclic we set our flag to 1
        if (current_length == 0)
        {
            quit = 1;
        }
    }

    myshm->terminate = 1;
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

    if (shm_unlink(SHM_NAME) == -1)
    {
        fprintf(stderr, "[%s] ERROR: shm-unlink failed: %s\n", program_name, strerror(errno));
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

    if (sem_unlink(SEM_2) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(SEM_3) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(SEM_4) == -1)
    {
        fprintf(stderr, "[%s] ERROR: sem_close failed: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}