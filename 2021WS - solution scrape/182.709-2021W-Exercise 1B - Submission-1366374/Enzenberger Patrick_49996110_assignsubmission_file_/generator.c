/**
 * @file generator.c
 * @author Patrick Enzenberger <e11709517@student.tuwien.ac.at>
 * @date 14.11.2021
 * 
 * @brief generates feedback arc sets based on given graph  
 * 
 * @details This program repeatedly generates a random feedback arc set. 
 * It is written to a shared memory buffer. Only solutions with 8 or less edges are considered.
 * 
 * @param EDGE, both indices of an edge seperated by an '-'
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
#include <time.h>
#define SHM_NAME "/11709517_myshm"
#define SEM_BUFFER_FREE "/11709517_mysem_free"
#define SEM_BUFFER_USED "/11709517_mysem_used"
#define SEM_BUFFER_WRITE "/11709517_mysem_write"
#define MAX_EDGE_IN_BUFFER 8
#define MAX_SOL_IN_BUFFER (4096 / MAX_EDGE_IN_BUFFER)

char *myprog = "generator";

// stores feedback arc set
struct myshm_circular_buffer
{
    int write_pos;                                              // position to be written to next
    int solution_num_nodes[MAX_SOL_IN_BUFFER];                  // number of nodes needed to be removed
    char solution_node1[MAX_SOL_IN_BUFFER][MAX_EDGE_IN_BUFFER]; //first index of node an edge is associated with. These edges need to be removed
    char solution_node2[MAX_SOL_IN_BUFFER][MAX_EDGE_IN_BUFFER]; //second index of node an edge is associated with. These edges need to be removed
    int exit;                                                   // flag to indicate the generator processes need to quit
};

void usage(void);
void myerror(char *text);
void cleanup(int shmfd, sem_t *sem_free, sem_t *sem_used, sem_t *sem_write,
             struct myshm_circular_buffer *myshm_cb);

int main(int argc, char *argv[])
{

    int node_count = 0;  // number of nodes in the graph
    int edge_from[argc]; //first index of node an edge is associated with
    int edge_to[argc];   //second index of node an edge is associated with
    char *node1 = NULL;  //stores one node at a time
    char *node2 = NULL;  //stores one node at a time
    int i = 0;
    while (i < argc - 1)
    {
        // looking for highest index to determine number of nodes
        node1 = strtok(argv[i + 1], "-");
        node2 = strtok(NULL, "-");
        if ((strtok(NULL, "-") != NULL) | (node1 == NULL) | (node2 == NULL))
        {
            myerror("wrong edge input format.");
        }
        //indecies of nodes of the associated edges get stored
        edge_from[i] = strtol(node1, NULL, 10);
        edge_to[i] = strtol(node2, NULL, 10);

        if (edge_from[i] + 1 > node_count)
        {
            node_count = edge_from[i] + 1;
        }
        if (edge_to[i] + 1 > node_count)
        {
            node_count = edge_to[i] + 1;
        }

        i++;
    }
    //at least one edge needed
    if (node_count < 1)
    {
        usage();
        myerror("no valid edges");
    }

    //open shared memory object:
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1)
    {
        myerror("shared memory open failed");
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

    // set up semaphores
    sem_t *sem_free = sem_open(SEM_BUFFER_FREE, 0);
    if (sem_free == SEM_FAILED)
    {
        cleanup(shmfd, sem_free, NULL, NULL, myshm_cb);
        myerror("semaphore SEM_BUFFER_FREE failed");
    }
    sem_t *sem_used = sem_open(SEM_BUFFER_USED, 0);
    if (sem_used == SEM_FAILED)
    {
        cleanup(shmfd, sem_free, NULL, NULL, myshm_cb);
        myerror("semaphore SEM_BUFFER_USED failed");
    }
    sem_t *sem_write = sem_open(SEM_BUFFER_WRITE, 0);
    if (sem_write == SEM_FAILED)
    {
        cleanup(shmfd, sem_free, sem_used, NULL, myshm_cb);
        myerror("semaphore SEM_BUFFER_WRITE failed");
    }

    srand(time(NULL)*getpid());
    int permutation[node_count];
    for (int i = 0; i < node_count; i++)
    {
        permutation[i] = i;
    }

    char solution_edge_from[MAX_EDGE_IN_BUFFER]; //first index of node an edge is associated with
    char solution_edge_to[MAX_EDGE_IN_BUFFER];   //second index of node an edge is associated with
    int solution_edge_count;                     // number of edges needed to be removed
    while (myshm_cb->exit != 1)
    {

        solution_edge_count = 0;
        
        //generate permutation
        for (int i = 0; i < node_count; i++)
        {
            int r = rand() % (i + 1);
            int temp = permutation[r];
            permutation[r] = permutation[i];
            permutation[i] = temp;
        }

        int pos_u;
        int pos_v;
        //select vertices with u<v
        for (int i = 0; i < node_count; i++)
        {

            for (int j = 0; j < node_count; j++)
            {
                if (permutation[j] == edge_from[i])
                {
                    pos_u = j;
                }
                if (permutation[j] == edge_to[i])
                {
                    pos_v = j;
                }
                
                
            }
            
            //int pos_u = permutation[edge_from[i]];
            //int pos_v = permutation[edge_to[i]];
            if (pos_u > pos_v)
            {
                solution_edge_from[solution_edge_count] = edge_from[i];
                solution_edge_to[solution_edge_count] = edge_to[i];
                solution_edge_count++;
            }
        }

        // if solution has too many edges;
        if (solution_edge_count > MAX_EDGE_IN_BUFFER)
        {
            continue;
        }

        // check semaphores before writing to buffer
        if (sem_wait(sem_write) == -1)
        {
            cleanup(shmfd, sem_free, sem_used, sem_write, myshm_cb);
            myerror("sem_wait(sem_write) error");
        }
        if (sem_wait(sem_free) == -1)
        {
            cleanup(shmfd, sem_free, sem_used, sem_write, myshm_cb);
            myerror("em_wait(sem_free) error");
        }

        // write valid solution to buffer
        if (myshm_cb->exit == 1)
        {
            break;
        }
        for (int i = 0; i < solution_edge_count; i++)
        {
            myshm_cb->solution_node1[myshm_cb->write_pos][i] = solution_edge_from[i];
            myshm_cb->solution_node2[myshm_cb->write_pos][i] = solution_edge_to[i];
        }
        myshm_cb->solution_num_nodes[myshm_cb->write_pos] = solution_edge_count;
        myshm_cb->write_pos++;
        myshm_cb->write_pos %= MAX_SOL_IN_BUFFER;

        // ajust semaphores after writing to buffer
        if (sem_post(sem_used) == -1)
        {
            cleanup(shmfd, sem_free, sem_used, sem_write, myshm_cb);
            myerror("sem_post(sem_used) error");
        }
        if (sem_post(sem_write) == -1)
        {
            cleanup(shmfd, sem_free, sem_used, sem_write, myshm_cb);
            myerror("sem_post(sem_write) error");
        }
    }
    cleanup(shmfd, sem_free, sem_used, sem_write, myshm_cb);
    return EXIT_SUCCESS;
}

void usage(void)
{
    fprintf(stderr, "Usage: %s EDGE1...\n", myprog);
}

void myerror(char *text)
{
    fprintf(stderr, "[%s] ERROR: %s:\n", myprog, text);
    exit(EXIT_FAILURE);
}

/**
 * @brief clean up all used ressources
 */
void cleanup(int shmfd, sem_t *sem_free, sem_t *sem_used, sem_t *sem_write,
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
