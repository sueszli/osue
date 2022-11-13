/**
 * @file supervisor.c
 * @author Michael Gahleitner, 01633034
 * @brief Run a supervisor for 3 coloring graph
 * @details Run supervisor for 3 coloring graph which reads the shared memory and always saves
 * the best result for a 3 coloring graph. Terminates when the graph is acyclic.
 * @date 12.11.2021
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "globals.h"
#include "shm_data.h"
#include "sem_util.h"

sem_t *sem_free_space;
sem_t *sem_used_space;
sem_t *sem_write_index;

circular_buffer_t *shared_memory;
edge_t best_solution[9];
int read_index;
int *shared_write_index;
int shared_memory_fd;
int shared_write_index_fd;

/**
 * @brief Prints usage
 * @details Prints usage which displays the usage of options and arguments and exits with EXIT_FAILURE status.
 */
void usage(void)
{
    fprintf(stderr, "Usage: %s\n", program_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Close fd
 * @details Closes file descriptor and prints error, if occured during closing
 * @param fd file descriptor
 */
void handle_close(int fd)
{
    if (close(fd) == -1)
    {
        fprintf(stderr, "%s ERROR: close failed: %s\n", program_name, strerror(errno));
    }
}

/**
 * @brief Unlinks shared memory
 * @details Unlinks shared memory and prints error, if occured during unlinking
 * @param shm shared memory
 */
void handle_shm_unlink(char *shm)
{
    if (shm_unlink(shm) == -1)
    {
        fprintf(stderr, "%s ERROR: shm_unlink failed: %s\n", program_name, strerror(errno));
    }
}

/**
 * @brief Frees the memory
 * @details Unmaps closes and unlinks shared memory. Closes and unlinks semaphores
 */
void free_memory(void)
{
    if (munmap(shared_memory, sizeof *shared_memory) == -1)
    {
        fprintf(stderr, "%s ERROR: munmap failed: %s\n", program_name, strerror(errno));
    }
    if (munmap(shared_write_index, sizeof *shared_write_index) == -1)
    {
        fprintf(stderr, "%s ERROR: munmap failed: %s\n", program_name, strerror(errno));
    }

    handle_close(shared_memory_fd);
    handle_close(shared_write_index_fd);

    handle_shm_unlink(SHARED_MEM);
    handle_shm_unlink(WRITE_INDEX);

    handle_sem_close(sem_free_space, program_name);
    handle_sem_close(sem_used_space, program_name);
    handle_sem_close(sem_write_index, program_name);

    handle_sem_unlink(FREE_SPACE, program_name);
    handle_sem_unlink(USED_SPACE, program_name);
    handle_sem_unlink(WRITE_INDEX, program_name);
}

/**
 * @brief Error exit
 * @details Frees memory and exits with EXIT_FAILURE
 */
void exit_error(void)
{
    free_memory();
    exit(EXIT_FAILURE);
}

/**
 * @brief Reads new solution
 * @details Reads solution from shared memory and saves it if it's a better
 * than the currenlty saved one. Prints the newly saved solution.
 */
void read_solution(void)
{
    printf("Solution with %d edges:", shared_memory->solutions[read_index][0].to);
    for (int i = 0; i < shared_memory->solutions[read_index][0].to; i++)
    {
        best_solution[i].from = shared_memory->solutions[read_index][i + 1].from;
        best_solution[i].to = shared_memory->solutions[read_index][i + 1].to;
        printf(" %d-%d", best_solution[i].from, best_solution[i].to);
    }
    printf("\n");
}

/**
 * @brief Inits semaphore and shared memory
 * @details Initializes shared memory and semaphores while also handling possible
 * errors during initiation.
 */
void init_sem_shm(void)
{
    sem_write_index = sem_open(WRITE_INDEX, O_CREAT | O_EXCL, 0600, 1);
    sem_free_space = sem_open(FREE_SPACE, O_CREAT | O_EXCL, 0600, MAX_BUFFER);
    sem_used_space = sem_open(USED_SPACE, O_CREAT | O_EXCL, 0600, 0);

    shared_memory_fd = shm_open(SHARED_MEM, O_CREAT | O_RDWR, 0600);
    if (shared_memory_fd == -1)
    {
        fprintf(stderr, "%s ERROR: shm_open failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
    if (ftruncate(shared_memory_fd, sizeof *shared_memory) == -1)
    {
        fprintf(stderr, "%s ERROR: ftruncate failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
    shared_memory = mmap(NULL, sizeof *shared_memory, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    if (shared_memory == MAP_FAILED)
    {
        fprintf(stderr, "%s ERROR: mmap failed: %s\n", program_name, strerror(errno));
        exit_error();
    }

    shared_write_index_fd = shm_open(WRITE_INDEX, O_CREAT | O_RDWR, 0600);
    if (shared_write_index_fd == -1)
    {
        fprintf(stderr, "%s ERROR: shm_open failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
    if (ftruncate(shared_write_index_fd, sizeof *shared_write_index) == -1)
    {
        fprintf(stderr, "%s ERROR: ftruncate failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
    shared_write_index = mmap(NULL, sizeof *shared_write_index, PROT_READ | PROT_WRITE, MAP_SHARED, shared_write_index_fd, 0);
    if (shared_write_index == MAP_FAILED)
    {
        fprintf(stderr, "%s ERROR: mmap failed: %s\n", program_name, strerror(errno));
        exit_error();
    }
}

/**
 * @brief Handles signal
 * @details Sets stop flag to 1, frees memory and exits.
 */
void handle_signal(int signal)
{
    shared_memory->stop = 1;
    free_memory();
    exit(EXIT_SUCCESS);
}

/**
 * @brief Program entry
 * @details starts supervisor and reads from shared memory
 * @param argc argument count
 * @param argv argument vector
 */
int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        // Terminate and show usage
    }
    else
    {
        program_name = argv[0];

        struct sigaction sa;
        sa.sa_handler = handle_signal;
        sigaction(SIGINT, &sa, NULL);

        init_sem_shm();

        shared_write_index = 0;

        read_index = 0;
        best_solution[0].to = -1;
        shared_memory->stop = 0;
        while (shared_memory->stop == 0)
        {
            sem_wait(sem_used_space);
            if (errno == EINTR)
            {
                fprintf(stderr, "%s EINTR - shutton down: %s\n", program_name, strerror(errno));
                exit_error();
            }
            if (shared_memory->solutions[read_index][0].to == 0)
            {
                shared_memory->stop = 1;
                printf("The graph is 3-colorable!\n");
            }
            else
            {
                if (best_solution[0].to == -1)
                {
                    read_solution();
                }
                else if (best_solution[0].to > shared_memory->solutions[read_index][0].to)
                {
                    read_solution();
                }
            }
            if (read_index < MAX_BUFFER_INDEX)
            {
                read_index++;
            }
            else
            {
                read_index = 0;
            }
            sem_post(sem_free_space);
        }

        free_memory();
    }

    exit(EXIT_SUCCESS);
}