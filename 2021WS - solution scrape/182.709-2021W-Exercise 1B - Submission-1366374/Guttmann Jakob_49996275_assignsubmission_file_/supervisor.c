/**
 * @file supervisor.c
 * @author Jakob Guttmann 11810289 <e11810289@student.tuwien.ac.at>
 * @brief reads data from shared memory and prints optimal solution to stdout
 * @details this module controlls all shared resources like semaphores or shared memory objects
 * and open and close these resources.
 * In addition this module contains a (small) signal handler function which just sets the volatile variable to 1
 * @date 11.11.2021
 * 
 * 
 */
#include "circbuf.h"
#include "graph.h"

/**
 * @brief stops while loop when signal occurs
 * 
 */
volatile sig_atomic_t quit = 0;

/**
 * @brief if signal SIGTERM or SIGINT is received this method is called by the signal handler
 * sets quit variable to 1
 * 
 * @param signal received signal (is not used)
 */
static void handle_signal(int signal);

/**
 * @brief main method for this module, reads out from shared memory and prints the best solution to stdout
 * @details at the beginning a signal handler is initialized
 * shared memory buffer in a form of a circular buffer is allocated and opened for all other processes
 * reads data from circular buffer and prints the minimal feedback arc set to stdout
 * ends if fb_arc set has size 0 (graph is acyclic) or if program receives SIGINT or a SIGTERM signal
 * message other processes that shared memory is off with active flag set to 0
 * int the end releases all allocated resources 
 * @param argc argument counter
 * @param argv arguments, but all arguments are ignored by the program
 * @return int 
 */
int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_signal;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);

    if (shmfd == -1)
        ERRORHANDLING("open shared memory")

    if (ftruncate(shmfd, sizeof(circbuf)) < 0)
    {
        close(shmfd);
        shm_unlink(SHM_NAME);
        ERRORHANDLING("truncate shared memory")
    }

    circbuf *shmbuf;

    shmbuf = mmap(NULL, sizeof(*shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (shmbuf == MAP_FAILED)
    {
        close(shmfd);
        shm_unlink(SHM_NAME);
        ERRORHANDLING("map shared memory")
    }

    srand(time(NULL));

    shmbuf->active = 1;
    shmbuf->rd_index = 0;
    shmbuf->wr_index = 0;
    shmbuf->minimum = 8;
    shmbuf->randomSeed = rand();

    sem_t *sem_mutex = sem_open(SEM_MX, O_CREAT | O_EXCL, 0600, 1);
    if (sem_mutex == SEM_FAILED)
    {
        munmap(shmbuf, sizeof(*shmbuf));
        close(shmfd);
        shm_unlink(SHM_NAME);
        ERRORHANDLING("open semaphore mutex")
    }

    sem_t *sem_wr = sem_open(SEM_WR, O_CREAT | O_EXCL, 0600, SIZE_BUF);
    if (sem_wr == SEM_FAILED)
    {
        munmap(shmbuf, sizeof(*shmbuf));
        close(shmfd);
        shm_unlink(SHM_NAME);
        sem_close(sem_mutex);
        sem_unlink(SEM_MX);
        ERRORHANDLING("open semaphore write")
    }

    sem_t *sem_rd = sem_open(SEM_RD, O_CREAT | O_EXCL, 0600, 0);
    if (sem_rd == SEM_FAILED)
    {
        munmap(shmbuf, sizeof(*shmbuf));
        close(shmfd);
        shm_unlink(SHM_NAME);
        sem_close(sem_mutex);
        sem_unlink(SEM_MX);
        sem_close(sem_wr);
        sem_unlink(SEM_WR);
        ERRORHANDLING("open semaphore read")
    }

    while ((shmbuf->minimum) != 0 && quit == 0)
    {
        if (sem_wait(sem_rd) == -1)
        {
            if (errno != EINTR)
            {
                munmap(shmbuf, sizeof(*shmbuf));
                close(shmfd);
                shm_unlink(SHM_NAME);
                sem_close(sem_mutex);
                sem_unlink(SEM_MX);
                sem_close(sem_wr);
                sem_unlink(SEM_WR);
                sem_close(sem_rd);
                sem_unlink(SEM_RD);
                ERRORHANDLING("wait sem_rd")
            }
            else
            {
                break;
            }
        }

        arcset_t arcst = shmbuf->buffer[(int)shmbuf->rd_index];
        int num_edges = arcst.num_edges;
        if (num_edges < (shmbuf->minimum))
        {
            printArcset(arcst.edges, num_edges, argv[0]);
            printf("\n");
            shmbuf->minimum = num_edges;
        }
        shmbuf->rd_index = (shmbuf->rd_index + 1) % SIZE_BUF;
        if (sem_post(sem_wr) == -1)
        {
            munmap(shmbuf, sizeof(*shmbuf));
            close(shmfd);
            shm_unlink(SHM_NAME);
            sem_close(sem_mutex);
            sem_unlink(SEM_MX);
            sem_close(sem_wr);
            sem_unlink(SEM_WR);
            sem_close(sem_rd);
            sem_unlink(SEM_RD);
            ERRORHANDLING("post sem_wr")
        }
    }
    shmbuf->active = 0;

    if (munmap(shmbuf, sizeof(*shmbuf)) == -1)
    {
        close(shmfd);
        shm_unlink(SHM_NAME);
        sem_close(sem_mutex);
        sem_unlink(SEM_MX);
        sem_close(sem_wr);
        sem_unlink(SEM_WR);
        sem_close(sem_rd);
        sem_unlink(SEM_RD);
        ERRORHANDLING("unmap shared memory")
    }

    if (close(shmfd) == -1)
    {
        shm_unlink(SHM_NAME);
        sem_close(sem_mutex);
        sem_unlink(SEM_MX);
        sem_close(sem_wr);
        sem_unlink(SEM_WR);
        sem_close(sem_rd);
        sem_unlink(SEM_RD);
        ERRORHANDLING("close shared memory")
    }

    if (shm_unlink(SHM_NAME) == -1)
    {
        sem_close(sem_mutex);
        sem_unlink(SEM_MX);
        sem_close(sem_wr);
        sem_unlink(SEM_WR);
        sem_close(sem_rd);
        sem_unlink(SEM_RD);
        ERRORHANDLING("unlink shared memory")
    }

    if (sem_close(sem_mutex) == -1)
    {
        sem_unlink(SEM_MX);
        sem_close(sem_wr);
        sem_unlink(SEM_WR);
        sem_close(sem_rd);
        sem_unlink(SEM_RD);
        ERRORHANDLING("close semaphore mutex")
    }
    if (sem_close(sem_wr) == -1)
    {
        sem_unlink(SEM_WR);
        sem_close(sem_rd);
        sem_unlink(SEM_RD);
        sem_unlink(SEM_MX);
        ERRORHANDLING("close semaphore write")
    }
    if (sem_close(sem_rd) == -1)
    {
        sem_unlink(SEM_RD);
        sem_unlink(SEM_MX);
        sem_unlink(SEM_WR);
        ERRORHANDLING("close semaphore rd")
    }

    if (sem_unlink(SEM_MX) == -1)
    {
        sem_unlink(SEM_MX);
        sem_unlink(SEM_WR);
        ERRORHANDLING("unlink semaphore mutex")
    }
    if (sem_unlink(SEM_RD) == -1)
    {
        sem_unlink(SEM_WR);
        ERRORHANDLING("unlink semaphore read")
    }
    if (sem_unlink(SEM_WR) == -1)
        ERRORHANDLING("unllink semaphore write")

    exit(EXIT_SUCCESS);
}

void handle_signal(int signal)
{
    quit = 1;
}