/**
 * @file generator.c
 * @author Michael Blank 11909459 <e11909459@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief Producer Program for feedback arc set problem.
 * 
 * @details This Program opens up a existing shared memory and 3 semaphore.
 * Whenever the Program gets access to the shared memory it generates a random verticies permutation
 * for its given Graph and writes all edges and selectes edges to be cut and writes them
 * into the circular buffer of the shared memory.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include "arcSet.h"

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details After writing the usage information the program will close with the EXIT_FAILURE value.
 * @param progName The name of the program.
 */
static void usage(char *progName) {
    fprintf(stderr, "Usage: %s EDGE1...\n", progName);
    exit(EXIT_FAILURE);
}

/**
 * Error helper function
 * @brief This function writes a message to stderr.
 * @details Given a message and a errnum this function will write the message and the corresponding error
 * message to stderr. Afterwards it will exit the program with the EXIT_FAILURE value.
 * @param progName The name of the program.
 */
static void error(char *progName, char *message ,int errnum) {
    fprintf(stderr, "%s: %s%s\n", progName, message, strerror(errnum));
    exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief The program starts here. This function implements the functionality of the program.
 * @details This function parses all the arguments opens the shared memory and semaphores.
 * It generates random permutations of the verticies given through the arguments and determines which of the edges to select.
 * When the function gets access to the shared memory it will write these edges into the circular buffer. 
 * When the state variable in the circular buffer is set to 1 the function will stop and the shared memory and the semaphores will be closed.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char *argv[]) {
    char *progName = argv[0];
    srand(getpid()); //Sets the seed of the random number generator

    if (argc < 2){
        usage(progName);
    }


    //Opening of Shared Memory and Semaphores
    int shmFd = shm_open(SHMNAME, O_RDWR, 0);
    if (shmFd == -1) {
        error(progName, "Shared Memory opening Error: ", errno);
    }

    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (myshm == MAP_FAILED) {
        close(shmFd);
        error(progName, "Shared Memory maping Error: ", errno);
    }

    sem_t *freeSem = sem_open(FREE_SEM, 0);
    if(freeSem == SEM_FAILED) {
        munmap(myshm, sizeof(*myshm));
        close(shmFd);
        error(progName, "Semaphore opening Error: ", errno);
    }
    sem_t *usedSem = sem_open(USED_SEM, 0);
    if(usedSem == SEM_FAILED) {
        sem_close(freeSem);
        munmap(myshm, sizeof(*myshm));
        close(shmFd);
        error(progName, "Semaphore opening Error: ", errno);
    }
    sem_t *writeSem = sem_open(WRITE_SEM, 0);
    if(writeSem == SEM_FAILED) {
        sem_close(usedSem);
        sem_close(freeSem);
        munmap(myshm, sizeof(*myshm));
        close(shmFd);
        error(progName, "Semaphore opening Error: ", errno);
    }

    //Main Part

    struct edge graph[argc-1];
    int vertexCount = 0;

    //Extracts the vertex information from the arguments
    for (int i = 0; i < argc - 1; i++) {
        char* ptr;
        graph[i].u = strtol(argv[i+1], &ptr, 10);
        ptr++;
        graph[i].v = strtol(ptr, &ptr, 10);

        if (graph[i].u > vertexCount) {
            vertexCount = graph[i].u;
        }
        if (graph[i].v > vertexCount) {
            vertexCount = graph[i].v;
        }
    }

    vertexCount++; // Before this vertexCount is index of the highest vertex (add vertex number 0);
    int permutation[vertexCount];

    while (myshm->state == 0) {

        //Create randomized permutation
        for (int i = 0; i < vertexCount; i++) {
            permutation[i] = i;
        }
        for (int i = 0; i < vertexCount; i++) {
            int randI = rand() % vertexCount;
            int temp = permutation[i];
            permutation[i] = permutation[randI];
            permutation[randI] = temp;
        }

        struct edge toCut[argc-1];
        memset(&toCut, 0, sizeof(toCut));
        int cutCounter = 0;

        for (int i = 0; i < argc - 1; i++) {
            
            for (int j = 0; j < vertexCount; j++)
            {
                if (permutation[j] == graph[i].u) {
                    break;
                }

                if (permutation[j] == graph[i].v) {
                    toCut[cutCounter] = graph[i];
                    cutCounter++;
                    break;
                }
            }
        }

        if (cutCounter <= MAX_EDGES) {

            sem_wait(writeSem);
            if (myshm->state)
                break;
            sem_wait(freeSem);

            myshm->buf[myshm->wr_pos].count = cutCounter;

            for (int i = 0; i < cutCounter; i++) {
                myshm->buf[myshm->wr_pos].edges[i] = toCut[i];
            }
            myshm->wr_pos += 1;
            myshm->wr_pos %= sizeof(myshm->buf) / sizeof(struct bufEntry);
            sem_post(usedSem);
            sem_post(writeSem);
        }
    }

    //Closing of Shared Memory and Semaphores

    sem_close(writeSem);
    sem_close(usedSem);
    sem_close(freeSem);

    munmap(myshm, sizeof(*myshm));
    close(shmFd);

    return EXIT_SUCCESS;
}