/**
 * @file generator.c
 * @author Jakob Guttmann 11810289 <e11810289@student.tuwien.ac.at>
 * @brief writes data (arcsets) to shared memory and computes possible arcsets
 * @details this module computes different arcsets and writes to the shared circular buffer
 * opens shared resources and do not need to free the shared resources (is done by supervisor)
 * gets as input the edges
 * @date 11.11.2021
 * 
 * 
 */
#include "circbuf.h"
#include "graph.h"

#include <regex.h>
#include <sys/types.h>
#include <time.h>

/**
 * @brief size of array storing all nodes in the beginning
 * 
 */
#define NODESLENGTH 128

/**
 * @brief checks the given input of the program, ends program if input is wrong
 * @details checks all entries of argv[] (except argv[0]) if the entries have the form
 * of src-dest whereby src and dest are natural numbers
 * if this is not the case method return NULL pointer
 * fills Edge vector with the given edges by argv and saves the amount of edges without multiple edges in outIndex
 * also saves the amount of nodes in outHighestNode pointer
 * returns pointer of all nodes in optimal size (size = number of nodes in graph)
 * 
 * @param argc argument counter (how many entries are in argv)
 * @param argv argument vector (what are the arguments for the program)
 * @param edges has to be already allocated edge vector of minimum size of (argc-1). Method saves all edges in the vector
 * @param nodes saves input nodes in vector, length of vector is saved in address of pointer outHighestNode
 * @param outEdges writes the number of edges in the int variable 
 * @param outHighestNode as input the actual length of nodes, writes the number of nodes in this variable
 */
static int *checkInput(int argc, char *argv[], Edge *edges, int *nodes, int *outEdges, int *outHighestNode);

/**
 * @brief main method of generator, process edges from input, compute new feedback_arc_sets and writes in shared memory
 * @details checks input for correctness. if input is wrong formatted program exit with error code EXIT_FAILURE
 * then opens all shared resources like shared memory and semaphores
 * generate new random seed and shuffle vertices
 * perform monte carlo algorithm and gets an feedback arc_set, algorithm gets also global minimum and stops if arcset is greater than actual minimum
 * (performance reasons)
 * if actual arcset is smaller than global minimum then write into circular buffer
 * then continue until active flag is 0 or minimum is 0
 * end closes all resources locally
 * 
 * @param argc argument counter
 * @param argv argument vector, arguments of program must be in form of src-dest whereby src and dest are natural numbers
 * @return int exit value, 0 means successful, otherwise an error ocurred
 */
int main(int argc, char *argv[])
{

    int numEdges = argc - 1;

    int highestNode = NODESLENGTH;

    int *nodes = malloc(sizeof(int) * highestNode);
    if (nodes == NULL)
        ERRORHANDLING("allocate nodes vector")

    Edge *edges = malloc(sizeof(Edge) * numEdges);
    if (edges == NULL)
    {
        ERRORHANDLING("allocate edges vector")
    }

    int *temp;

    temp = checkInput(argc, argv, edges, nodes, &numEdges, &highestNode);

    if (temp == NULL)
    {
        free(edges);
        ERRORHANDLING("handle input. Probably wrong format or allocation failed.")
    }
    nodes = temp;

    if (numEdges != (argc - 1))
    {
        edges = realloc(edges, sizeof(Edge) * numEdges);
        if (edges == NULL)
        {
            free(nodes);
            free(edges);
            ERRORHANDLING("reallocate edges vector")
        }
    }

    //opening shared resources
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1)
    {
        free(nodes);
        free(edges);
        ERRORHANDLING("open shared memory")
    }

    circbuf *shmbuf;

    shmbuf = mmap(NULL, sizeof(*shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (shmbuf == MAP_FAILED)
    {
        free(nodes);
        free(edges);
        close(shmfd);
        ERRORHANDLING("map shared memory")
    }

    sem_t *sem_mutex = sem_open(SEM_MX, 0);
    if (sem_mutex == SEM_FAILED)
    {
        free(nodes);
        free(edges);
        close(shmfd);
        munmap(shmbuf, sizeof(*shmbuf));
        ERRORHANDLING("open semaphore mutex")
    }

    sem_t *sem_wr = sem_open(SEM_WR, 0);
    if (sem_wr == SEM_FAILED)
    {
        free(nodes);
        free(edges);
        close(shmfd);
        munmap(shmbuf, sizeof(*shmbuf));
        sem_close(sem_mutex);
        ERRORHANDLING("open semaphore write")
    }

    sem_t *sem_rd = sem_open(SEM_RD, 0);
    if (sem_rd == SEM_FAILED)
    {
        free(nodes);
        free(edges);
        close(shmfd);
        munmap(shmbuf, sizeof(*shmbuf));
        sem_close(sem_mutex);
        sem_close(sem_wr);
        ERRORHANDLING("open semaphore read")
    }

    //new seed to get different results
    if (sem_wait(sem_mutex) == -1)
    {
        free(edges);
        free(nodes);
        close(shmfd);
        munmap(shmbuf, sizeof(*shmbuf));
        sem_close(sem_mutex);
        sem_close(sem_wr);
        sem_close(sem_rd);
        ERRORHANDLING("wait sem mutex")
    }
    srand(shmbuf->randomSeed);
    shmbuf->randomSeed = rand();
    if (sem_post(sem_mutex) == -1)
    {
        free(edges);
        free(nodes);
        close(shmfd);
        munmap(shmbuf, sizeof(*shmbuf));
        sem_close(sem_mutex);
        sem_close(sem_wr);
        sem_close(sem_rd);
        ERRORHANDLING("wait sem mutex")
    }

    Edge arcst[8];
    int count_arcset = 1;

    //writing into circular buffer
    while ((shmbuf->minimum) != 0 && shmbuf->active)
    {

        permutate(nodes, highestNode);
        count_arcset = feedback_arc_set(numEdges, highestNode, shmbuf->minimum, nodes, edges, arcst);

        if (count_arcset == -1)
            continue;
        if (count_arcset == -2)
        {
            free(edges);
            free(nodes);
            close(shmfd);
            munmap(shmbuf, sizeof(*shmbuf));
            sem_close(sem_mutex);
            sem_close(sem_wr);
            sem_close(sem_rd);
            ERRORHANDLING("wrong nodes in feedback_arc_set")
        }

        //if smaller then write in buffer
        if (count_arcset < shmbuf->minimum)
        {

            if (sem_wait(sem_wr) == -1)
            {
                free(edges);
                free(nodes);
                close(shmfd);
                munmap(shmbuf, sizeof(*shmbuf));
                sem_close(sem_mutex);
                sem_close(sem_wr);
                sem_close(sem_rd);
                ERRORHANDLING("wait sem_wr")
            }
            if (sem_wait(sem_mutex) == -1)
            {
                free(edges);
                free(nodes);
                close(shmfd);
                munmap(shmbuf, sizeof(*shmbuf));
                sem_close(sem_mutex);
                sem_close(sem_wr);
                sem_close(sem_rd);
                ERRORHANDLING("wait sem_mutex")
            }

            int wr_index = shmbuf->wr_index;
            shmbuf->buffer[wr_index].num_edges = count_arcset;
            Edge *arcsetEdges = shmbuf->buffer[wr_index].edges;

            for (int i = 0; i < count_arcset; i++)
            {
                arcsetEdges[i].dest = arcst[i].dest;
                arcsetEdges[i].src = arcst[i].src;
            }
            shmbuf->wr_index = (wr_index + 1) % SIZE_BUF;
            if (sem_post(sem_mutex) == -1)
            {
                free(edges);
                free(nodes);
                close(shmfd);
                munmap(shmbuf, sizeof(*shmbuf));
                sem_close(sem_mutex);
                sem_close(sem_wr);
                sem_close(sem_rd);
                ERRORHANDLING("post sem_mutex")
            }
            if (sem_post(sem_rd) == -1)
            {
                free(edges);
                free(nodes);
                close(shmfd);
                munmap(shmbuf, sizeof(*shmbuf));
                sem_close(sem_mutex);
                sem_close(sem_wr);
                sem_close(sem_rd);
                ERRORHANDLING("post sem_rd")
            }
        }
    }

    free(nodes);
    free(edges);

    if (munmap(shmbuf, sizeof(*shmbuf)) == -1)
    {
        close(shmfd);
        sem_close(sem_mutex);
        sem_close(sem_wr);
        sem_close(sem_rd);
        ERRORHANDLING("unmap shared memory")
    }

    if (close(shmfd) == -1)
    {
        sem_close(sem_mutex);
        sem_close(sem_wr);
        sem_close(sem_rd);
        ERRORHANDLING("close shared memory")
    }

    if (sem_close(sem_mutex) == -1)
    {
        sem_close(sem_wr);
        sem_close(sem_rd);
        ERRORHANDLING("close semaphore mutex")
    }
    if (sem_close(sem_wr) == -1)
    {
        sem_close(sem_rd);
        ERRORHANDLING("close semaphore write")
    }
    if (sem_close(sem_rd) == -1)
        ERRORHANDLING("close semaphore rd")
}

static int *checkInput(int argc, char *argv[], Edge *edges, int *nodes, int *outIndex, int *outHighestNode)
{
    regex_t regex;
    int check;
    int startNode, endNode;
    int index = 0;
    int countNodes = 0;
    int length = *outHighestNode;

    check = regcomp(&regex, "^[0-9]+-[0-9]+$", REG_EXTENDED | REG_NOSUB);
    if (check)
    {
        regfree(&regex);
        return NULL;
    }

    for (int i = 1; i < argc; i++)
    {
        check = regexec(&regex, argv[i], 0, NULL, 0);
        if (check == REG_NOMATCH)
        {
            regfree(&regex);
            free(nodes);
            return NULL;
        }

        check = sscanf(argv[i], "%d-%d", &startNode, &endNode);
        if (check == EOF || check != 2)
        {
            regfree(&regex);
            free(nodes);
            return NULL;
        }

        check = 1;
        for (int j = 0; j < (i - 1); j++)
        {
            if (edges[j].src == startNode && edges[j].dest == endNode)
            {
                check = 0;
                break;
            }
        }
        if (!check)
            continue;

        edges[index].src = startNode;
        edges[index].dest = endNode;

        index++;

        check = search(nodes, startNode, countNodes);
        if (check == -1)
        {
            if (countNodes == length)
            {
                length *= 2;
                nodes = realloc(nodes, sizeof(int) * length);
                if (nodes == NULL)
                    return NULL;
            }
            nodes[countNodes] = startNode;
            countNodes++;
        }
        check = search(nodes, endNode, countNodes);
        if (check == -1)
        {
            if (countNodes == length)
            {
                length *= 2;
                nodes = realloc(nodes, sizeof(int) * length);
                if (nodes == NULL)
                    return NULL;
            }
            nodes[countNodes] = endNode;
            countNodes++;
        }
    }
    regfree(&regex);

    *outIndex = index;
    *outHighestNode = countNodes;

    nodes = realloc(nodes, sizeof(int) * countNodes);
    return nodes;
}