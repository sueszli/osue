/**
 * @file generator.c
 * @author Nicholas Harisch 11705361<harischnicholas@gmail.com>
 * @date 12.11.2021
 *
 * @brief Generator generating solutions for 3-color
 * 
 * This program generates 3-color solutions from a set of edges provided as program arguments by randomly coloring nodes and removing edges from
 * neighbouring same-colored nodes. Solutions <= 8 edges are suggested to the
 * supervisor through shared memory. Synchronization is provided
 * by using semaphores.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "supervisor.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>

struct Ring_Buffer *ringbuffer; /**< Ringbuffer in the shared memory */
int shmfd;                      /**< File descriptor */
sem_t *sem_free;                /**< Semaphore keeping track of free space in ring buffer */
sem_t *sem_used;                /**< Semaphore keeping track of used space in ring buffer */
sem_t *sem_block;               /**< Semaphore to synch. writing to ring buffer */
int *adjacencyMatrix;
static void splitEdge(int *e, char *edge);
static void paintNodesRandomly(char *nodeColors, ssize_t size);
static void removeNeighbours(int *adjacencyMatrixm, ssize_t size, char nodeColors[], struct Solution *result);

/**
 * Cleanup function.
 * @brief This function cleans up shared memory handling and semaphores.
 * @details global variables: sem_free, sem_used, sem_block, shmfd, ringbuffer
 */
void cleanUp()
{   
    //close file descriptor
    if (close(shmfd) == -1)
    {
        exit(EXIT_FAILURE);
    }

    //unmap shared memory object
    if (munmap(ringbuffer, sizeof(struct Ring_Buffer)) == -1)
    {
         exit(EXIT_FAILURE);
    }


    sem_close(sem_free);
    sem_close(sem_block);
    sem_close(sem_used);
}



/**
 * Program entry point.
 * @brief The program starts here. This function takes care of managing the creating of 3-color solutions of a given graph.
 * @details Handles signal handling, opening  semaphores, access to the memory ring buffer and then loops (if semaphores allow) to
 * generate new random 3-color solution. If a solution created is smaller than the SOLUTION_MAX, it is written into the shared memory buffer
 * global variables: sem_free, sem_used, sem_block, shmfd, ringbuffer
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[])
{

    signal(SIGINT, cleanUp);
    signal(SIGTERM, cleanUp);

    //Open semaphores by name
    sem_free = sem_open(SEM_FREE, 0);
    sem_used = sem_open(SEM_USED, 0);
    sem_block = sem_open(SEM_BLOCK, 0);

    //Terminate if no arguments are given
    if (argc < 2)
    {
        printf("Pos. arguments missing");
        exit(EXIT_FAILURE);
    }

    int nodeAmount = 0;
    //Gets the amount of nodes in graph by looking for the highest number
    for (int i = 1; i < argc; i++)
    {
        fflush(stdout);
        int *edge = malloc(sizeof(int) * 2);
        if (edge == NULL)
        {
            exit(EXIT_FAILURE);
        }

        char temp[strlen(argv[i])];
        strcpy(temp, argv[i]);

        splitEdge(edge, temp);
        if (nodeAmount < *(edge))
        {
            nodeAmount = *(edge);
        }
        if (nodeAmount < *(edge + 1))
        {
            nodeAmount = *(edge + 1);
        }
        nodeAmount = nodeAmount +1;
        free(edge);
    }

    //Create or open shared mem obj
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        printf("shmfg failed \n");
        exit(EXIT_FAILURE);
    }

    //map shared memory object
    ringbuffer = mmap(NULL, sizeof(*ringbuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (ringbuffer == MAP_FAILED)
    {
        printf("map failed \n");
        exit(EXIT_FAILURE);
    }

    //As long as supervisor doesnt set status to 0, this loop keeps generating new 3 color solutions and puts them into the shared ringbuffer
    while ((*ringbuffer).status == 1)
    {
        //Build adjacency matrix
        adjacencyMatrix = (int*) malloc((nodeAmount) * (nodeAmount)*sizeof(int)); //< 2d adjacency array / matrix /*
       
        //Fills adj matrix according to the edges between nodes
        for (int i = 1; i < argc; i++)
        {
            int *edge = malloc(sizeof(int) * 2);
            splitEdge(edge, argv[i]);
            adjacencyMatrix[(nodeAmount) * (*(edge)) + *(edge+1)] = 1;
            adjacencyMatrix[(nodeAmount) * (*(edge+1)) + *(edge)] = 1;
            free(edge);
        }

        //Generate random colors for every node
        char nodeColors[nodeAmount]; //< holds the color for every node/*
        paintNodesRandomly(nodeColors, sizeof(nodeColors));

        //removes all edges between neighbours with same color
        struct Solution sol; //< holds the current solution/*
        removeNeighbours(adjacencyMatrix, nodeAmount, nodeColors, &sol);
        //If size is smaller than accepted max solution size, it is put in the ring buffer
        if (sol.size <= SOLUTION_MAX)
        {
            sem_wait(sem_block);
            sem_wait(sem_free);
            (*ringbuffer).solutions[(*ringbuffer).wr_pos] = sol;

            (*ringbuffer).wr_pos = (*ringbuffer).wr_pos + 1;
            (*ringbuffer).wr_pos = (*ringbuffer).wr_pos % MAX_SIZE;
            sem_post(sem_block);
            sem_post(sem_used);
        }

    }
    free(adjacencyMatrix);
    cleanUp();
    exit(EXIT_SUCCESS);
}

/**
 * Split Edge string function.
 * @brief Splits string of edge into the two nodes
 * @param e pointer to the node result array
 * @param edge pointer to the edge string to split
 */
static void splitEdge(int *e, char *edge)
{
    char *ptr;
    char cpyedge[strlen(edge)];
    strcpy(cpyedge, edge);
    char *token = strtok(cpyedge, "-");
    //Error handling if not right format (edge) TODO
    *e = strtol(token, &ptr, 10);
    token = strtok(NULL, "-");
    *(e + 1) = strtol(token, &ptr, 10);
}

/**
 * Random color function.
 * @brief asserts 3 different chars (r,g,b) randomly to char array
 * @param nodeColors pointer to the color result array
 * @param size how many colors to generate, size of nodeColors
 */
static void paintNodesRandomly(char *nodeColors, ssize_t size)
{
    char colors[3] = {'r', 'g', 'b'};
    srand(time(NULL));
    for (int i = 0; i < size; i++)
    {
        *(nodeColors + i) = colors[rand() % 3];
    }
}


/**
 * Remove neighbours function
 * @brief Removes neighbours with same color
 * @details Goes through half of the adjacency matrix (diagonal) and for every edge its finds, it checks if the two nodes have the same color, if yes
 * it removes the edge and adds it to the solution struct
 * @param adjacencyMatrixm pointer to the adjacency matrix
 * @param size size of one one dimension of the adjacency matrix
 * @param nodeColors node color array
 * @param result array for the result (removed edges)
 */
static void removeNeighbours(int *adjacencyMatrixm, ssize_t size, char nodeColors[], struct Solution *result)
{
    //max 8 edges rem
    (*result).size = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (*(adjacencyMatrixm + j * size + i) == 1)
            {
                if (nodeColors[i] == nodeColors[j])
                {
                    (*result).edges[(*result).size].a = i;
                    (*result).edges[(*result).size].b = j;
                    (*result).size = (*result).size + 1;
                }
            }
        }
    }
}
