/**
 * @file supervisor.c
 * @author 12024755, Florin-Elis Buju <e12024755@student.tuwien.ac.at> 
 * @date 08.11.2021
 * @brief Betriebssysteme 1B Feedback Arc Set supervisor
 * @details The program is setting up a shared memory and semaphores. It is recieving Arc Sets from generator(s)
 * until it gets a special Signal or finds a right solution. 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>  

#include <fcntl.h> 
#include <stdio.h> 
#include <sys/mman.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <semaphore.h>
#include <signal.h>

#define SHM_NAME "/12024755sharedmem"  /** Name of shared memory */
#define MAX_SIZE (32) /** Max Size of Circular Buffer */
#define STOP_VALUE (7200) /** Defines the value of the Stop Signal for Generators */
#define MAX_EDGES (8) /** Max Edges for an Arcset */

#define SEM_1 "/12024755sem_1" /** Name from "Free Space" Semaphore */
#define SEM_2 "/12024755sem_2" /** Name from "Used Space" Semaphore */
#define SEM_3 "/12024755sem_3" /** Name from "mutually exclusive access" Semaphore */

typedef struct {int x, y;} edge; /** struct of an single edge */
typedef struct {edge edges[MAX_EDGES]; int size;} arcSet; /** struct of an single edge */

static int shmid = -1; /** shared memeory for circual buffer */
static arcSet *mapshm = NULL; /** ArcSet array as circual buffer, used to write or read */
static sem_t *free_sem = NULL; /** Free Space Semaphore, free space in the circual buffer*/
static sem_t *used_sem = NULL; /** Used Space Semaphore, used space in the circual buffer*/
static sem_t *access_sem = NULL; /** Mutually exclusive access Semaphore, can only write mutually exclusive to the buffer individually */
static volatile sig_atomic_t quit = 0; /** Signal handeling of SIGTERM and SIGINT*/


static bool cleanupError = false; /** Shows if the clean up prozess had an Error and program will not excute futher */

static void myErrorout(char *failedto, char *program); 

/**
 * @brief Cleanup function, removes all semaphore and mmaps and shared memorys
 * @details Funktion is called when program terminates or had an error, 
 * @param programname is giving to call further errors if necessary
 * @return no return value (void)  
 **/ 
static void cleanUp(char *program)
{
    // Clear and unlink all semaphores, mmaps and shared memorys
    if(mapshm != NULL)
    {
        if (munmap(mapshm, sizeof(*mapshm)) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to unmap the mapped memory.",program);
        }
    }

    if(shmid != -1)
    {
        if (close(shmid) < 0)
        {
            cleanupError = true;
            myErrorout("Closing the shared memory failed.",program);
        }
        if (shm_unlink(SHM_NAME) == -1)
        {
            myErrorout("Faild to unlink the shared memory.",program); 
        }
    }

    if(free_sem != NULL)
    {
        if (sem_close(free_sem) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to clean up Semephore free_sem.",program);
        }
        if (sem_unlink(SEM_1) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to unlink Semephore free_sem.",program);
        }
    }
    if(used_sem != NULL)
    {
        if (sem_close(used_sem) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to clean up Semephore used_sem.",program);
        }
        if (sem_unlink(SEM_2) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to unlink Semephore used_sem.",program);
        }
    }
    if(access_sem != NULL)
    {
        if (sem_close(access_sem) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to clean up Semephores access_sem.",program);
        }
        if (sem_unlink(SEM_3) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to unlink Semephore access_sem.",program);
        }
    }
}


/**
 * @brief Error function of the program
 * @details Will output the passed error message and will exit the programm with EXIT_FAILURE
 * @param failedto is the ErrorMessage which should be print, and program is the program name
 * @return no return value (void) 
 **/
static void myErrorout(char *failedto, char *program)
{
    if(strcmp(strerror(errno),"Success")==0)
    {
        
        fprintf(stderr, "[%s] ERROR: %s\n", program, failedto);
    }
    else
    {
        
        fprintf(stderr, "[%s] ERROR: %s: %s\n", program, failedto, strerror(errno));
    }

    if(cleanupError == false)
    {
        cleanUp(program);
    }
    exit(EXIT_FAILURE);
}
/**
 * @brief Standard handle signal methode
 * @details Will be executed as soon as a signal is received by the sigaction, sets var quit to one -> stops while
 * @param Signal int is received but not used in this methode. Must be a paramameter. A signal handler is defined that way.
 * @return no return value (void) 
 **/
static void handle_signal(int signal)
{
    quit = 1;
}
/**
 * @brief Error function of the program
 * @details Will output the passed error message and will exit the programm with EXIT_FAILURE
 **/
int main(int argc, char *argv[]) 
{
    char *program = argv[0];
    // Setting up an shared memory
    shmid = shm_open(SHM_NAME, O_RDWR | O_CREAT,0600);
    if(shmid == -1)
    {
        myErrorout("Failed to creat a shared memory.",program); 
    }
    // truncate the shared memory to a specific size
    if(ftruncate(shmid, sizeof(arcSet) * MAX_SIZE) < 0)
    {
        myErrorout("Failed to set size of shared memory.",program); 
    }

    
    mapshm = mmap(NULL, sizeof(arcSet) * (MAX_SIZE+1), PROT_READ | PROT_WRITE,MAP_SHARED, shmid, 0);
    if (mapshm == MAP_FAILED)
    {
        myErrorout("Failed to map memory.",program); 
    }
    // Set the write counter to 0
    int writestart = 0;
    memcpy(&mapshm[MAX_SIZE].size,&writestart,sizeof(int));

    //Open Semaphores
    free_sem = sem_open(SEM_1, O_CREAT | O_EXCL, 0600, MAX_SIZE); //Free Space
    used_sem = sem_open(SEM_2, O_CREAT | O_EXCL, 0600, 0); //Used Space
    access_sem = sem_open(SEM_3, O_CREAT | O_EXCL, 0600, 1); //mutually exclusive access
    if(free_sem == SEM_FAILED)
    {
        myErrorout("Could not open SEM_1.",program); 
    }
    if(used_sem == SEM_FAILED)
    {
        myErrorout("Could not open SEM_2.",program); 
    }
    if(access_sem == SEM_FAILED)
    {
        myErrorout("Could not open SEM_3.",program); 
    }

    // Setting up the signals for SIGINT and SIGTERM
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int readpos = 0;
    arcSet leastEdges;
    leastEdges.size = MAX_EDGES;
    // Check incoming solutions until a signal is reveived or until it is found out that the graph is acyclic 
    while(!quit)
    {
        if (sem_wait(used_sem) == -1) {
            if (errno == EINTR)
            {
                continue;
            } 
            myErrorout("An error accured while wating for semaphore used_sem.",program); 
        }
        // Get a generated Arcset from the circular buffer at position readpos
        arcSet value = mapshm[readpos];

        if (sem_post(free_sem) == -1) 
        {
            myErrorout("An error accured while wating for semaphore free_sem.",program); 
        }

        readpos +=1;
        readpos %=MAX_SIZE;
        // Check if new the new arcset is "better" than the old one
        if(value.size < leastEdges.size)
        {
            leastEdges = value;
            if(leastEdges.size == 0)
            {
                printf("\n[%s] The graph is acyclic!",program);
                break;
            }
            printf("\n[%s] Solution with %d edges:",program,leastEdges.size);
            int s;
            for (s = 0; s < leastEdges.size; s++)
            {
                printf(" %d-%d",leastEdges.edges[s].x,leastEdges.edges[s].y);             
            }
        }                  
    }

    // Tell generators to stop
    int writepos1 = STOP_VALUE;
    memcpy(&mapshm[MAX_SIZE].edges[0].x,&writepos1,sizeof(int));
    // In special case, also tell waiting generators to stop waiting
    int cc;
    for (cc = 0; cc < MAX_SIZE; cc++)
    {
        sem_post(free_sem);
    }
    
    // Clean up the program and exit
    cleanUp(program);
    printf("\nSupervisor completed!\n");
    exit(EXIT_SUCCESS);
}