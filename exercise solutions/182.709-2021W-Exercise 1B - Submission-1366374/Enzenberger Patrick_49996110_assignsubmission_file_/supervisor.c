/**
 * @file supervisor.c
 * @author Patrick Enzenberger <e11709517@student.tuwien.ac.at>
 * @date 14.11.2021
 * 
 * @brief reads solutions to a feedback arc set problem from shared memory and prints best solution so far.  
 * 
 * @details sets up shared memory for generator programm to write solutions to. 
 * If a better solution is found it is printed. When there is a solution with zero edges to be removed
 * the programm terminates after a -The graph is acyclic!- message.
 * 
 **/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#define SHM_NAME "/11709517_myshm"
#define SEM_BUFFER_FREE "/11709517_mysem_free"
#define SEM_BUFFER_USED "/11709517_mysem_used"
#define SEM_BUFFER_WRITE "/11709517_mysem_write"
#define MAX_EDGE_IN_BUFFER 8
#define MAX_SOL_IN_BUFFER (4096 / MAX_EDGE_IN_BUFFER)

char *myprog = "supervisor";
volatile sig_atomic_t quit = 0;

// stores solutions to a feedback arc set problem
struct myshm_circular_buffer
{
    int write_pos;                                              // position to be written to next
    int solution_num_nodes[MAX_SOL_IN_BUFFER];                  // number of nodes needed to be removed
    char solution_node1[MAX_SOL_IN_BUFFER][MAX_EDGE_IN_BUFFER]; //first index of node an edge is associated with. These edges need to be removed
    char solution_node2[MAX_SOL_IN_BUFFER][MAX_EDGE_IN_BUFFER]; //second index of node an edge is associated with. These edges need to be removed
    int exit;                                                   // flag to indicate the generator processes need to quit
};

static void usage(void);
static void myerror(char *text);
static void handle_signal(int signal) { quit = 1; }
static void cleanup(int shmfd, sem_t *sem_free, sem_t *sem_used, sem_t *sem_write,
             struct myshm_circular_buffer *myshm_cb);

int main(int argc, char *argv[])
{
    if (optind < argc)
    {
        usage();
        myerror("did not expect arguments");
    }

    //signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //create and/or open the shared memory object:
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        myerror("shared memory open failed");
    }

    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct myshm_circular_buffer)) < 0)
    {
        close(shmfd);
        unlink(SHM_NAME);
        myerror("shared memory ftruncate failed");
    }

    // initialise and map shm
    struct myshm_circular_buffer *myshm_cb;
    myshm_cb = mmap(NULL, sizeof(*myshm_cb), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm_cb == MAP_FAILED)
    {
        close(shmfd);
        unlink(SHM_NAME);
        myerror("shared memory mmap failed");
    }
    myshm_cb->write_pos = 0;

    // set up semaphores
    //monitors free space
    sem_t *sem_free = sem_open(SEM_BUFFER_FREE, O_CREAT | O_EXCL, 0600, MAX_SOL_IN_BUFFER);
    if (sem_free == SEM_FAILED)
    {
        cleanup(shmfd, NULL, NULL, NULL, myshm_cb);
        myerror("semaphore SEM_BUFFER_FREE failed");
    }
    //monitors used space
    sem_t *sem_used = sem_open(SEM_BUFFER_USED, O_CREAT | O_EXCL, 0600, 0);
    if (sem_used == SEM_FAILED)
    {
        cleanup(shmfd, sem_free, NULL, NULL, myshm_cb);
        myerror("semaphore SEM_BUFFER_USED failed");
    }
    //prevents collisions when writing to buffer
    sem_t *sem_write = sem_open(SEM_BUFFER_WRITE, O_CREAT | O_EXCL, 0600, 1);
    if (sem_write == SEM_FAILED)
    {
        cleanup(shmfd, sem_free, sem_used, NULL, myshm_cb);
        myerror("semaphore SEM_BUFFER_WRITE failed");
    }

    int read_pos = 0;       //position where supervisor currently reads from buffer
    char best_node1[8];     //first index of node an edge is associated with
    char best_node2[8];     //second index of node an edge is associated with
    int best_num_nodes = 9; // stores number of edges needed to be removed in the best solution so far

    // save and print best solution in buffer so far
    while (quit != 1)
    {
        if (sem_wait(sem_used) == -1)
        {
            if (errno == EINTR)
                continue;
            cleanup(shmfd, sem_free, sem_used, sem_write, myshm_cb);
            myerror("sem_wait(sem_used) error");
        }

        // if new best solution
        if (myshm_cb->solution_num_nodes[read_pos] < best_num_nodes)
        {
            // if graph is acyclic
            if (myshm_cb->solution_num_nodes[read_pos] == 0)
            {
                fprintf(stdout, "The graph is acyclic!\n");
                break;
            }

            //save new best solution and print
            best_num_nodes = myshm_cb->solution_num_nodes[read_pos];
            fprintf(stdout, "Solution with %d edges: ", best_num_nodes);
            for (int i = 0; i < best_num_nodes; ++i)
            {
                best_node1[i] = myshm_cb->solution_node1[read_pos][i];
                best_node2[i] = myshm_cb->solution_node2[read_pos][i];
                fprintf(stdout, "%d-%d, ", best_node1[i], best_node2[i]);
            }
            fprintf(stdout, "\n");
        }

        if (sem_post(sem_free) == -1)
        {
            cleanup(shmfd, sem_free, sem_used, sem_write, myshm_cb);
            myerror("sem_post(sem_free) error");
        }

        read_pos++;
        read_pos %= MAX_SOL_IN_BUFFER;
    }

    myshm_cb->exit = 1; //notify other generator processes to quit
    cleanup(shmfd, sem_free, sem_used, sem_write, myshm_cb);

    return EXIT_SUCCESS;
}

static void usage(void)
{
    fprintf(stderr, "Usage: %s\n", myprog);
}

static void myerror(char *text)
{
    fprintf(stderr, "[%s] ERROR: %s:\n", myprog, text);
    exit(EXIT_FAILURE);
}

/**
 * @brief clean up all used ressources
 * @param shmfd, three semaphores, cicular buffer
 */
static void cleanup(int shmfd, sem_t *sem_free, sem_t *sem_used, sem_t *sem_write,
             struct myshm_circular_buffer *myshm_cb)
{
    if (sem_free != NULL)
    {
        sem_close(sem_free);
        sem_unlink(SEM_BUFFER_FREE);
    }
    if (sem_used != NULL)
    {
        sem_close(sem_used);
        sem_unlink(SEM_BUFFER_USED);
    }
    if (sem_write != NULL)
    {
        sem_close(sem_write);
        sem_unlink(SEM_BUFFER_WRITE);
    }
    munmap(myshm_cb, sizeof(*myshm_cb));
    close(shmfd);
    shm_unlink(SHM_NAME);
}
