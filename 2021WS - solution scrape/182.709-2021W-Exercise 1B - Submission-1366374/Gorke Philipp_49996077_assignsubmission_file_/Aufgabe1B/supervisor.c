/**
 * @file supervisor.c
 * @author Philipp Gorke <e12022511@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief supervisor main programm
 * 
 * This programm sets up shared mem and semaphores and reads from the buffer
 *
 **/


#include "helpFunctions.h"

static int shmfd = -1;      //shared Memory file descriptor
static mySharedMem *myshm = NULL;   //shared Memory
static sem_t *s1 = NULL;    //semaphore 1
static sem_t *s2 = NULL;    //semaphore 2
static sem_t *s3 = NULL;    //semaphore 3

static void initSHM(void);
static void initSEM(void);
static void closeSHM(void);
static void closeSEM(void);
static void handler(int num);


/**
 * @brief reads the circular buffer and prints the result
 * @details First shared memory and semaphores are initialized
 * second, the programm begins to read the circular buffer and 
 * store the results, aswell as compares them to recent results
 * if the best result = 0 the programm terminates and the generator
 * is stopped aswell. 
 */
int main(int argc, char *argv[])
{   
    if (argc > 1) {
        printf("Too many arguments: only call supervisor");
        exit(EXIT_FAILURE);
    }

    initSHM();
    initSEM();

    graph currentSolution;
    int solutionNumber;
    int bestsolution = -1;

    //circular Buffer
    while (true)
    {
        signal(SIGINT, handler); 
        signal(SIGTERM, handler);
        sem_wait(s2);
        currentSolution = myshm->graphs[myshm->rd_pos];
        solutionNumber = currentSolution.nmb;
        sem_post(s1);
        myshm->rd_pos += 1;
        myshm->rd_pos %= 900;

        //checks if the new solution is better than the old one
        if (bestsolution == -1 || solutionNumber < bestsolution)
        {
            bestsolution = solutionNumber;
            // solution == 0 means the graph is 3-colorable and the programm terminates
            if (bestsolution == 0)
            {
                myshm->kill = true;
                printf("%s", "The graph is 3-colorable!");
                return 0;
            }
            printf("Solution with %d edges: ", bestsolution);
            for (size_t i = 0; i < bestsolution; i++)
            {
                printf("%d - %d ", currentSolution.edges[i].v1, currentSolution.edges[i].v2);
            }
            printf("\n");
        }
    }
    return 0; 
}


/**
 * @brief starts the shared memory
 * @details initiazes atexit, starts and links the shared Memory
 */
static void initSHM(void)
{
    atexit(closeSHM);
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shmfd < 0)
    {
        printf("%s", "ERROR: SHMFD could not be opened");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmfd, sizeof(mySharedMem)) < 0)
    {
        printf("%s", "ERROR: SHMFD could not be truncated");
        exit(EXIT_FAILURE);
    }
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED)
    {
        printf("%s", "ERROR: shared memory could not be mapped");
        exit(EXIT_FAILURE);
    }
    if (close(shmfd) < 0)
    {
        printf("%s", "ERROR: shmfd could not be closed");
        exit(EXIT_FAILURE);
    }
    shmfd = -1;

    myshm->rd_pos = 0;
    myshm->wr_pos = 0;
    myshm->kill = false;
}


/**
 * @brief starts semaphores
 * @details initiazes atexit, starts and links the semaphores
 */
static void initSEM(void)
{
    atexit(closeSEM);
    s1 = sem_open(SEM_1, O_CREAT | O_EXCL, 0600, 900);
    if (s1 == SEM_FAILED)
    {
        printf("%s", "ERROR: Semaphore 1 could not be opened.");
        exit(EXIT_FAILURE);
    }
    s2 = sem_open(SEM_2, O_CREAT | O_EXCL, 0600, 0);
    if (s2 == SEM_FAILED)
    {
        printf("%s", "ERROR: Semaphore 2 could not be opened.");
        exit(EXIT_FAILURE);
    }
    s3 = sem_open(SEM_3, O_CREAT | O_EXCL, 0600, 1);
    if (s3 == SEM_FAILED)
    {
        printf("%s", "ERROR: Semaphore 3 could not be opened.");
        exit(EXIT_FAILURE);
    }
}


/**
 * @brief closes shared Memory
 * @details stops and unlinks the sharedMemory
 */
static void closeSHM(void)
{
    //stops the generators
    if (myshm != NULL)
        myshm->kill = true;

    //unmaps shared Memory
    if (myshm != NULL)
    {
        if (munmap(myshm, sizeof(*myshm)) == -1)
        {
            printf("%s", "ERROR: Shared memory could not be unmapped");
        }
    }
    //unlink shared Memory
    if (shm_unlink(SHM_NAME) == -1)
    {
        printf("%s", "ERROR: Shared memory could not be unlinked");
    }

    if (shmfd != -1)
    {
        close(shmfd);
        shmfd = -1;
    }
}


/**
 * @brief closes semaphores
 * @details stops and unlinks the semaphores
 */
static void closeSEM(void)
{

    if (sem_close(s1) == -1)
    {
        printf("%s", "ERROR: Semaphore1 could not be closed");
    }
    if (sem_close(s2) == -1)
    {
        printf("%s", "ERROR: Semaphore2 could not be closed");
    }
    if (sem_close(s3) == -1)
    {
        printf("%s", "ERROR: Semaphore3 could not be closed");
    }
    if (sem_unlink(SEM_1) == -1)
    {
        printf("%s", "ERROR: Semaphore1 could not be unlinked");
    }
    if (sem_unlink(SEM_2) == -1)
    {
        printf("%s", "ERROR: Semaphore2 could not be unlinked");
    }
    if (sem_unlink(SEM_3) == -1)
    {
        printf("%s", "ERROR: Semaphore2 could not be unlinked");
    }
}

/**
 * @brief handels signals
 * @details programm exits and atexit is executed - therefore everything closes
 */
static void handler(int nmb) {
    exit(EXIT_FAILURE);
}

