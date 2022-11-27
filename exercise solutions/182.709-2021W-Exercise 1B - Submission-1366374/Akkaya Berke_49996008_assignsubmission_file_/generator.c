/**
 * @file generator.c
 * @author Berke Akkaya (11904656)
 * @brief my implentation of a generator of random solutions to the feedback arc set problem 
 * @date 2021-11-14
 * 
 * @details the generator is given a set of edges when it is started. The programm reads the input and makes an edges array out of it
 * After that the programm uses the Fishers Yates algorithm to make random permutations of the edge set. After that the solution to the problem is calculated by
 * searching for the conflicting positions inside of the permutations where u>v (u,v)->resembling an edge. If the solution has less edges than the stated limit, which
 * is specified in myheader.h then the solution is written onto the buffer. When the programm terminates all the buffer and semaphores are terminating as well. 
 */

#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <limits.h>
#include "myheader.h"
#include <stdlib.h>
#include <time.h>

static sem_t *sem_free = NULL;  //semaphorpointer that keeps track of the free space inside of the buffer
static sem_t *sem_used = NULL;  //semaphorpointer that keeps track of the used space inside of the buffer
static sem_t *sem_mutex = NULL; //semaphorpointer that keeps track of the process that are trying to write onto the circular buffer
static myshms *buf = NULL;      //pointer to the shared memory/circular buffer
int shmfd;

static void clean_buffer(void);
static void errorHandling(const char *errorMessage)
{
    fprintf(stderr, "[./generator]: %s\n ", errorMessage);
    clean_buffer();
    exit(EXIT_FAILURE);
}
/**
 * @brief this function reads the input that has been given nad checks if the input is also correct
 * 
 * @param argc has to be bigger than 1 
 * @param argv has the edges information stored
 * @param graph is going to be returning the an graph that we can use in main
 */
void inputedges(int *argc, char **argv, edge *graph)
{

    for (int i = 1; i < *argc; i++)
    {
        int edgestart;
        int edgeend;
        edge newEdge;
        if ((sscanf(argv[i], "%d-%d", &edgestart, &edgeend) < 2))
        {
            errorHandling("Please use the format edge-edge");
        }
        if (edgestart < 0 || edgeend < 0)
        {
            errorHandling("Please only use positive values for the edges");
        }
        newEdge.startedge = edgestart;
        newEdge.endedge = edgeend;
        graph[i - 1] = newEdge;
    }
}
/**
 * @brief searches for the position of an element inside of a permutation
 * 
 * @param c is the value that we search for
 * @param Perm the array that the permutation is stored in
 * @param verticescount is the length of the permutationsarray
 * @return int gives us the position of the searched element. If it can not be found -1 (In our case that will not happen)
 */
int findInPerm(int c, int Perm[], int verticescount)
{
    int lengthP = verticescount;

    for (size_t i = 0; i < lengthP; i++)
    {
        if (Perm[i] == c)
        {
            return i;
        }
    }
    return -1;
}
/**
 * @brief uses the Fisher Yates algorithm to create a permutation
 * 
 * @param permutation the array in which the permutation will be stored in 
 * @param n the length of the array
 */
void permutations(int *permutation, int n)
{
    for (int i = 0; i < n; i++)
    {
        permutation[i] = i;
    }

    int j;
    int tmp;
    for (int i = 0; i < n + 1; i++)
    {
        j = rand() % n;
        tmp = permutation[j];
        permutation[j] = permutation[i];
        permutation[i] = tmp;
    }
}

/**
 * @brief maps a shared memory and opens semaphores used for the synchronization
 * 
 * @details gloabl variable: buf
 * @details global variable: sem_free
 * @details global variable: sem_used
 * @details global variable: sem_mutex
 */
void loadsharedmemory()
{
    shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if ((shmfd < 0))
    {
        //ausgabe error: shm open failed
    }
    if ((buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == MAP_FAILED)
    {
        //ausgabe error
    }
    sem_free = sem_open(SEM_FREE_NAME, BUFFER_SIZE);
    if (sem_free == SEM_FAILED)
    {
        //error sem free
    }
    sem_used = sem_open(SEM_USED_NAME, 0);
    if (sem_used == SEM_FAILED)
    {
        //error sem used
    }
    sem_mutex = sem_open(SEM_MUTEX_NAME, 1);
    if (sem_mutex == SEM_FAILED)
    {
        //error sem mutex
    }
}

/**
 * @brief calculates a feedback arc set like it is specified on the excercise sheet
 * 
 * @param graph is the graph in which has been read from the input
 * @param graph_len is the amount of the edges in the graph
 * @param vertexPerm is permutation that has been created in main
 * @param vertices number of vertices in the graph
 * @return solutions 
 */
static solutions solver(edge *graph, int graph_len, int Perm[], int vertices)
{
    int counter = 0;
    solutions solution;
    for (size_t i = 0; i < graph_len; i++)
    {
        if (findInPerm(graph[i].startedge, Perm, vertices) > findInPerm(graph[i].endedge, Perm, vertices))
        {
            solution.edgesolution[counter] = graph[i];
            counter++;
        }
    }
    solution.amount = counter;
    return solution;
}
/**
 * @brief counts the amount of vertices in the graph 
 * 
 * @param graph the graph which has been read from input
 * @param length the length resembles the amount of edges that has been read from the input
 * @return int returns the value of the max Node and adds 1
 */
int vertexCount(edge *graph, int length)
{
    int max = 0;

    for (size_t i = 0; i < length; i++)
    {
        if (graph[i].startedge > max)
        {
            max = graph[i].startedge;
        }
        if (graph[i].endedge > max)
        {
            max = graph[i].endedge;
        }
    }
    return max;
}

void clean_buffer()
{
    if (munmap(buf, sizeof(*buf)) == -1)
    {
        errorHandling("munmap failed in clean buffer for buf");
    }
    close(shmfd);
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
    fprintf(stdout, "cleaned up in generator\n");
}

/**
 * @brief sets up the input graph and uses the methods to find a solution to the feedback arc problem and writes solutions to the circular buffer until it 
 * is notified to stop
 * 
 * @param argc argument counter
 * @param argv argument array
 * 
 */
int main(int argc, char **argv)
{

    edge *graph = malloc(sizeof(edge) * (argc - 1));
    loadsharedmemory();

    if (argc == 1)
    {
        exit(EXIT_FAILURE);
    }

    inputedges(&argc, argv, graph);
    int maxintvertex = vertexCount(graph, argc - 1);
    while (buf->state == 0)
    {

        solutions trys;
        int permutation[maxintvertex + 1];
        permutations(permutation, maxintvertex + 1);
        trys = solver(graph, argc - 1, permutation, maxintvertex);

        if (trys.amount < MAX_RESULT_EDGES)
        {
            for (size_t i = 0; i < trys.amount; i++)
            {
                fprintf(stdout, "(%d-%d)", trys.edgesolution[i].startedge, trys.edgesolution[i].endedge);
            }
            fprintf(stdout, "\n");
        }
        if (buf->state == 1)
        {
            continue;
        }

        if (sem_wait(sem_free) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                errorHandling("Problem in freewait");
            }
        }
        if (sem_wait(sem_mutex) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                errorHandling("Problem in mutexwait");
            }
        }
        buf->buffersolution[buf->writepos] = trys;
        buf->writepos += 1;
        buf->writepos %= BUFFER_SIZE;
        if (sem_post(sem_mutex) == -1)
        {
            errorHandling("Problem in mutexpost");
        };

        if (sem_post(sem_used) == -1)
        {
            errorHandling("problem in usedpost");
        };
    }
    clean_buffer();
    exit(EXIT_SUCCESS);
}
