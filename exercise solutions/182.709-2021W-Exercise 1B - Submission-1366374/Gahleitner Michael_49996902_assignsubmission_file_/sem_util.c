#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/**
 * @brief Closes semaphore
 * @details Closes semaphore and prints error, if occured during unlinking
 * @param sem file descriptor
 */
void handle_sem_close(sem_t *sem, char *program_name)
{
    if (sem_close(sem) == -1)
    {
        fprintf(stderr, "%s ERROR: sem_close failed: %s\n", program_name, strerror(errno));
    }
}

/**
 * @brief Unlinks semaphore
 * @details Unlinks semaphore and prints error, if occured during unlinking
 * @param sem file descriptor
 */
void handle_sem_unlink(char *sem, char *program_name)
{
    if (sem_unlink(sem) == -1)
    {
        fprintf(stderr, "%s ERROR: sem_unlink failed: %s\n", program_name, strerror(errno));
    }
}