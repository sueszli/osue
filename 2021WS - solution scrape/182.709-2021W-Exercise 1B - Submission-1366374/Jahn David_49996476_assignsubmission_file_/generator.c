/**
 * @file generator.c
 * @author David Jahn, 12020634
 * @brief The generator program
 * @version 1.0
 * @date 2021-11-14
 * 
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "circular_buffer.h"

#define SHM_CIRC_BUFFER "12020634_circ_buf1"
#define MY_SEM_READ "12020634_sem_read1"
#define MY_SEM_WRITE "12020634_sem_write1"
/**
 * @brief Programm name
 * 
 */
const char *PROGRAM_NAME;

/**
 * @brief Semaphore for writing in the buffer
 * 
 */
sem_t *writeSem;
/**
 * @brief Semaphore for reading out of the buffer
 * 
 */
sem_t *readSem;

/**
 * @brief Shared memory circular Buffer
 * 
 */
circular_buffer_t *buffer;
/**
 * @brief Shared memory file descriptor
 * 
 */
int shmfd;

/**
 * @brief Method to close all Ressources if an error occures
 * 
 */
void closeRessourcesOnError(void)
{
    munmap(buffer, sizeof(*buffer));
    close(shmfd);
    //---- close semaphores
    sem_close(readSem);
    sem_close(writeSem);
}

/**
 * @brief A Mehtod to call if an error occures. It prints an error message and closes all ressources.
 * 
 * @param errorMsg Error Message to print
 */
void onError(char *errorMsg)
{
    fprintf(stderr, "An error occured in %s: \n", PROGRAM_NAME);
    fprintf(stderr, errorMsg);
    fprintf(stderr, "\n");
    fprintf(stderr, strerror(errno));
    fflush(stderr);
    closeRessourcesOnError();
    exit(EXIT_FAILURE);
}

/**
 * @brief Method to close all Ressources in a normal way.
 * 
 */
void closeRessourcesNormal(void)
{
    if (close(shmfd) == -1)
    {
        onError("Error while close of shm");
    }
    if (munmap(buffer, sizeof(*buffer)) == -1)
    {
        onError("Error while munmap");
    }
    //---- close semaphores
    if (sem_close(readSem) == -1)
    {
        onError("Error while sem_close");
    }
    if (sem_close(writeSem) == -1)
    {
        onError("Error while sem_close");
    }
}



/**
 * @brief Get a Random Number between 0 and 3
 * 
 * @return int A random Number
 */
int getRandomNumber()
{
    int lower = 0;
    int upper = 2;
    return (rand() % (upper - lower + 1)) + lower;
}

/**
 * @brief Calculates the egdes to remove to get a 3-colorable instance of a given Graph
 * 
 * @param input Graph to get 3-colored
 * @return graph_t Set of Egdes
 */
graph_t get3ColorableGraph(graph_t input)
{
    graph_t ret;
    ret.edges_len = 0;
    for (int i = 0; i < input.edges_len; i++)
    {
        if (input.edges[i].from.isColored == false)
        {
            input.edges[i].from.color = getRandomNumber();
            input.edges[i].from.isColored = true;
        }
        if (input.edges[i].to.isColored == false)
        {
            input.edges[i].to.color = getRandomNumber();
            input.edges[i].to.isColored = true;
        }
    }
    int counter = 0;
    for (int i = 0; i < input.edges_len; i++)
    {
        if (input.edges[i].from.color == input.edges[i].to.color)
        {
            ret.edges[counter] = input.edges[i];
            ret.edges_len++;
            counter++;
            input = removeEdge(input, input.edges[i]);
        }
    }
    return ret;
}

/**
 * @brief Method used for argument parsing
 * 
 * @param argc Argument counter
 * @param argv Array of arguments
 * @return graph_t Graph made out of command line arguments
 */
graph_t argumentParsing(int argc, char *argv[])
{
    //argument parsing
    graph_t inputGraph;
    inputGraph.edges_len = 0;
    char *tmp;
    int fromNode;
    int toNode;
    node_t from;
    node_t to;
    edge_t edge;
    char fromNodeChar[3];
    char toNodeChar[3];
    int graphCounter = 0;
    for (int i = optind; i < argc; i++)
    {
        tmp = argv[i];
        bool firstPart = true;
        int splitIdx = 0;
        for (int j = 0; j < strlen(tmp); j++)
        {
            if (tmp[j] == '-')
            {
                firstPart = false;
                continue;
            }
            if (firstPart)
            {
                fromNodeChar[j] = tmp[j];
            }
            else
            {
                toNodeChar[splitIdx] = tmp[j];
                splitIdx++;
            }
        }
        sscanf(fromNodeChar, "%d", &fromNode);
        sscanf(toNodeChar, "%d", &toNode);

        if (nodeExists(&inputGraph, fromNode))
        {
            from = getNode(&inputGraph, fromNode);
        }
        else
        {
            from.color = -1;
            from.id = fromNode;
            from.isColored = false;
        }
        if (nodeExists(&inputGraph, toNode))
        {
            to = getNode(&inputGraph, toNode);
        }
        else
        {
            to.color = -1;
            to.id = toNode;
            to.isColored = false;
        }
        edge.from = from;
        edge.to = to;
        inputGraph.edges_len++;
        inputGraph.edges[graphCounter] = edge;
        graphCounter++;
    }
    return inputGraph;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    PROGRAM_NAME = argv[0];
    shmfd = shm_open(SHM_CIRC_BUFFER, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        onError("Error while shm_open");
    }
    if (ftruncate(shmfd, sizeof(circular_buffer_t)) < 0)
    {
        onError("Error while ftruncate");
    }

    buffer = mmap(NULL, sizeof(circular_buffer_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buffer == MAP_FAILED)
    {
        onError("Error while mmap");
    }

    readSem = sem_open(MY_SEM_READ, 0);
    if (readSem == SEM_FAILED)
    {
        onError("Error while sem_open 1");
    }
    writeSem = sem_open(MY_SEM_WRITE, 0);
    if (writeSem == SEM_FAILED)
    {
        onError("Error while sem_open 2");
    }

    bool quit = false;
    graph_t initG = argumentParsing(argc, argv);
    graph_t outputGraph;
    int wr_pos = 0;
    while (!quit)
    {
        
        sem_wait(writeSem);
        quit = buffer->terminate;
        outputGraph = get3ColorableGraph(initG);   
        // printf("----------START");     
        // for (int i = 0; i < outputGraph.edges_len; i++)
        // {
        //     printf("\nfrom: %d, to %d\n", outputGraph.edges[i].from.id, outputGraph.edges[i].to.id);
        // }
        // printf("\nlen of g: %d", outputGraph.edges_len);
        // printf("----------END\n\n");
        buffer->graphs[wr_pos] = outputGraph;
        sem_post(readSem);
        wr_pos++;
        wr_pos %= MAX_ELEMENTS_IN_BUFFER;
    }
    closeRessourcesNormal();
    return 0;
}