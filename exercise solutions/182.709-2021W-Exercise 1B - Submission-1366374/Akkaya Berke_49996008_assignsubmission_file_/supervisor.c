/**
 * @file supervisor.c
 * @author Berke Akkaya 11904656
 * @brief this class has the task to find the best solution in the circular buffer, it reads the values that are written onto the buffer(from the generators)
 * 
 * @date 14.11.2021
 * 
 */
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <limits.h>
#include "myheader.h"
#include <stdlib.h>

static volatile sig_atomic_t terminate = 0; // this variable resembles a boolean value that tells the programm if it needs to terminate
static sem_t *sem_free = NULL;              //semaphorpointer that keeps track of the free space inside of the circular bufffer
static sem_t *sem_used = NULL;              //semaphorpointer that keeps track of the used space inside of the circular buffer
static sem_t *sem_mutex = NULL;             //semaphorpointer that doesnt let other processes overwrite solutions which have not been red
static myshms *buf = NULL;                  //pointer to the shared memory
int shmfd;
//initialized the smfd variable here in the case of an error

static void clean_buffer(void);

static void errorHandling(const char *errorMessage)
{
    fprintf(stderr, "[./supervisor]: %s\n ", errorMessage);
    clean_buffer();
    exit(EXIT_FAILURE);
}
void signalHandler(int signum)
{
    fprintf(stdout, "landed in the signalHandler\n");
    terminate = 1;
}
/**
 * @brief this method initializes the shared memory and the semaphores that are needed for the communication between the supervisor and the generators
 * @details global variable: buf
 * @details global variable: sem_free
 * @details global variable: sem_used
 */
static void initializesharedmemory()
{
    //initializes the shared memory
    //creates it or opens it
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        errorHandling("shm_open failed");
    }
    //sets the size of the shared memory
    if (ftruncate(shmfd, sizeof(struct myshm)) < 0)
    {
        errorHandling("ftruncate failed");
    }
    //maps the shared memory object
    buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (buf == MAP_FAILED)
    {
        errorHandling("mmap failed");
        //ausgabe error mmap hat failed
    }
    if (close(shmfd) == -1)
    {
        errorHandling("closing shmfd has failed");
    }
    //initialize the semaphores
    sem_unlink(SEM_FREE_NAME);
    sem_free = sem_open(SEM_FREE_NAME, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    sem_unlink(SEM_USED_NAME);
    sem_used = sem_open(SEM_USED_NAME, O_CREAT | O_EXCL, 0600, 0);
    sem_unlink(SEM_MUTEX_NAME);
    sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT | O_EXCL, 0600, 1);

    if (sem_free == SEM_FAILED)
    {
        errorHandling("sem_open of sem_free has failed");
    }
    if (sem_used == SEM_FAILED)
    {
        errorHandling("sem_open of sem_used has failed");
    }
    if (sem_mutex == SEM_FAILED)
    {
        errorHandling("sem_open of sem_mutex has failed");
    }
}

/**
 * @brief unlinks all the resources and closes them
 * 
 */
void clean_buffer()
{
    if (munmap(buf, sizeof(*buf)) == -1)
    {
        errorHandling("munmap failed in clean buffer for buf");
    }
    if (shm_unlink(SHM_NAME) == -1)
    {
        errorHandling("shm_ unlink failed in clean buffer for shm");
    }
    if (sem_close(sem_free) == -1)
    {
        errorHandling("sem_close failed in clean buffer for sem_free");
    }
    if (sem_close(sem_used) == -1)
    {
        errorHandling("sem_close failed in clean buffer for sem_used");
    }
    if (sem_close(sem_mutex) == -1)
    {
        errorHandling("sem_close failed in clean buffer for sem_mutex");
    }
    if (sem_unlink(SEM_FREE_NAME) == -1)
    {
        errorHandling("sem_unlink failed in clean buffer for SEM_FREE_NAME");
    }
    if (sem_unlink(SEM_USED_NAME) == -1)
    {
        errorHandling("sem_unlink failed in clean buffer for SEM_USED_NAME");
    }
    if (sem_unlink(SEM_MUTEX_NAME) == -1)
    {
        errorHandling("sem_unlink failed in clean buffer for SEM_MUTEX_NAME");
    }
    fprintf(stdout, "closed \n");
}

/**
 * @brief initiazes shared memory and semaphores;
 * reads the solutions from the circular buffer and looks for the best one
 */
int main(int argc, char **argv)
{
    //initialize signal handling
    struct sigaction actor;
    memset(&actor, 0, sizeof(actor));
    actor.sa_handler = signalHandler;
    if (sigaction(SIGINT, &actor, NULL) < 0)
    {
        errorHandling("sigaction has failed");
    }
    if (sigaction(SIGTERM, &actor, NULL) < 0)
    {
        errorHandling("sigaction has failed");
    }
    if (argc != 1)
    {
        errorHandling("It is not allowed to use any arguments in supervisor");
    }
    initializesharedmemory();
    buf->state = 0;
    buf->write_pos = 0;
    buf->readpos = 0;

    solutions bestsolution;
    bestsolution.amount = INT_MAX;

    while (!terminate)
    {
        if (sem_wait(sem_used) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                errorHandling("sem_wait has failed");
            }
        }
        solutions currentsolution = buf->buffersolution[buf->readpos];
        buf->readpos += 1;
        buf->readpos %= BUFFER_SIZE;

        if (buf->state == 0)
        {
            if (currentsolution.amount == 0)
            {
                fprintf(stdout, "the graph is acylic");
                terminate = 1;
                buf->state = 1;
            }

            else if (currentsolution.amount < bestsolution.amount)
            {
                bestsolution = currentsolution;
                fprintf(stdout, "Solution with %d edges:", bestsolution.amount);
                for (size_t i = 0; i < bestsolution.amount; i++)
                {
                    fprintf(stdout, "(%d-%d) ", bestsolution.edgesolution[i].startedge, bestsolution.edgesolution[i].endedge);
                }
                fprintf(stdout, "\n");
            }
        }

        sem_post(sem_free);
    }
    buf->state = 1;
    clean_buffer();
    exit(EXIT_SUCCESS);
}
