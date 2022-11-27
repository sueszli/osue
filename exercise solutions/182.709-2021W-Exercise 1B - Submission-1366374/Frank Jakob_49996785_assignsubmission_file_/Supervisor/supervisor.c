
/* ISSUES
makefile
*/


/*
*
* @file supervisor.c
* @author Jakob Frank (11837319)
* 
* @brief Reads solutions from circular buffer and checks for best solution
*
* @details The supervisor sets up the circular buffer and semaphores. It then
*          reads the by the generator processes created solutions from the circular
*          buffer and updates the best in real time. After an acyclic solution was reached
*          the supervisor sends all generator processes a signal to stop and stops itself       
*
* @date 12/11/2021
*/

#include "../circular_buffer.h"


char * programName;
volatile sig_atomic_t quit;

/*
*
* @brief a signal handler to exit the supervisor successfully
*
* @param signal the signal which the function prints
*
*/

void handle_signal(int signal);

/*
*
* @brief prints the current best solution to stdout
*
* @param result the result to be printed
*
*/

void printCurrentSolution(shm_t result);


/*
*
* @brief prints an edge to stdout
*
* @param e the edge to be printed
*
*/

void printEdge(edge e);

int main(int argc, char **argv)
{
    char * programName = argv[0];
    shm_t opt;

    //return error and exit in case of wrong usage

    if (argc > 1)
    {
        fprintf(stderr, "%s mustn't take any arguments", programName);
        exit(EXIT_FAILURE);
    }
    
    //init shared memory
    

    int fd;
    buf * bufptr;

    memInitSupervisor(&fd, &bufptr);

    bufptr -> quit = 0;
    
    //init circular buffer / semaphores (maybe replace with functions...)

    sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, BUF_SIZE); //write to buffer

    if (sem_free == SEM_FAILED)
    {
        fprintf(stderr, "Error, sem_free couldn't be opened by server\n");
        memClearSupervisor(&fd, &bufptr);
        sem_close(sem_free);
        exit(EXIT_FAILURE);
    }

    sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0); //read from buffer

    if (sem_used == SEM_FAILED)
    {
        fprintf(stderr, "Error, sem_used couldn't be opened by server\n");
        memClearSupervisor(&fd, &bufptr);
        sem_close(sem_used);
        exit(EXIT_FAILURE);
    }

    sem_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1); //ensure mutex (only 1 process can write in mem at a time)

    if (sem_write == SEM_FAILED)
    {
        fprintf(stderr, "Error, sem_write couldn't be opened by server\n");
        memClearSupervisor(&fd, &bufptr);
        sem_close(sem_write);
        exit(EXIT_FAILURE);
    }
    
    struct sigaction sa = {.sa_handler = handle_signal};
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    
    int * readPos = &(bufptr -> read_pos);
    int size = MAX_DATA;

    //repeatedly access the shared memory to check and update solutions

    while (quit == 0)
    {
        int sizeNew = readMem(readPos, bufptr, size, &opt); 

        if (sizeNew < size)
        {
            size = sizeNew;

            if (size == 0)
            {
                //sends signal to stop since graph is acyclic...

                fprintf(stdout, "The graph is acyclic!\n");

                break;
            }
            
            printCurrentSolution(opt);
        }
    }
    sem_close(sem_write);
    sem_close(sem_used);
    sem_close(sem_free);
    memClearSupervisor(&fd, &bufptr);

    bufptr -> quit = 1;

    fprintf(stderr, "Successfully caught exit signal %d, supervisor will terminate\n", quit);
    
    free(bufptr);

    exit(EXIT_SUCCESS);
    
    return 0;
}


void printCurrentSolution(shm_t result)
{
    fprintf(stdout, "This is a solution with %d edges: ", result.size);

    for (size_t i = 0; i < result.size; i++)
    {
        printEdge(result.edges[i]);
    }
    fprintf(stdout, "\n");
}

void printEdge(edge e)
{
    fprintf(stdout, "%d-%d, ",e.start, e.end);
}

void handle_signal(int signal)
{
    quit = 1;
}