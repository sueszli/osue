/**
 * @file util.h
 * @author Andreas Hoessl <e11910612@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Utility header.
 *
 * Variables, functions and structs both supervisor.c and generator.c use are declared here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>

#define SHM_NAME "/11910612_shm" /** < Name of the shared memory.*/
#define MAX_DATA (64) /** < The size of the circular buffer. */
#define ENTRY_SIZE (48) /** < The maximum size of a solution. */

#define SEM_FREE "/11910612_sem_free" /** < Name of the 'free' semaphore. */
#define SEM_USED "/11910612_sem_used" /** < Name of the 'used'' semaphore. */
#define SEM_MUTEX "/11910612_sem_mutex" /** < Name of the 'mutex' semaphore. */

char *myprog; /** < Used for storing the program name. */

struct myshm { /** < Structure of the circular buffer. */
    int state; /** < Flag for signaling the termination of the supervisor process to the generator. */
    char data[MAX_DATA][ENTRY_SIZE]; /** < The actual buffer. */
    int wr_pos; /** < Writing position for the generator. */
    int rd_pos; /** < Reading position for the supervisor. */
};

struct myshm *myshm; /** < The pointer to the shared memory. */

sem_t *sem_free; /** <  Semaphore to keep track of the free space in the buffer.*/
sem_t *sem_used; /** <  Semaphore to keep track of the used space in the buffer.*/
sem_t *sem_mutex; /** <  Semaphore for mutal exclusive access to the shared memory. */


/**
 * Procedure
 * @brief Failure exit with an error message.
 * @details A custom error message will be printed to 'stderr' and the program will terminate with 'EXIT_FAILURE'.
 * global variables: myprog
 * @param message error message
 */
void exit_error(char *message);

/**
 * Procedure
 * @brief Wait procedure for a semaphore..
 * @details This procedure calls 'sem_wait()'  and exits if there is an error.
 * @param semaphore the semaphore used
 */
void s_wait(sem_t *semaphore);

/**
 * Procedure
 * @brief Post procedure for a semaphore..
 * @details This procedure calls 'sem_post()'  and exits if there is an error.
 * @param semaphore the semaphore used
 */
void s_post(sem_t *semaphore);
