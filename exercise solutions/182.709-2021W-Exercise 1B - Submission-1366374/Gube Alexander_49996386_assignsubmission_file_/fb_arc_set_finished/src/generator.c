/**
 * @file generator.c
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief fb_arc_set generator module
 *
 * This part is one of the two major modules for the feedback arc set project, which aims
 * to find good solutions for the feedback arc set problem. This component is responsible fordate
 * generating solutions to the feeback arc problem by using a brute force approach. It is given a graph
 * and based on the input a feedback arc set with a maximum size of 8 is generated. A further description of the
 * creation of such solutions can be found in GraphUtils.h. When a solution is generated, it is reported to the supervisor
 * by writing it to the circularBuffer. Therefore shm and semaphore setup is needed as well (see also supervisor.c)
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>                   //for shared memory
#include <fcntl.h>

#include "ErrorHandling.h"
#include "SemUtils.h"
#include "ShmUtils.h"

#define PROG_NAME "generator"

/**
 * usage function.
 * @brief This function writes to stderr how the program is to be used
 */
static void usageMessage(void) {
    fprintf(stderr, "[%s] ERROR: invalid arguments, Usage: %s EDGE1...\n", PROG_NAME, PROG_NAME);
    exit(EXIT_FAILURE);
}

/**
 * swapping
 * @brief This function swaps two integer values
 * @param a pointer to the first int
 * @param b pointer to the second int
 * @details given two pointer to int variables, their values are swapped (e.g. in an array)
 */
static void swap (int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * Program entry point.
 * @brief This function is responsible for argument management, setting up the shm and its semaphores and afterwars generating solutions until it
 * is notified by the supervisor to terminate.
 * @details As no arguments are intended, it is terminated as far as parameters are provided with the usageMessage. Then the edges given to the generator are parsed.parameters
 * In order to generate feedback arc sets more efficiently, the edges are then converted to an adjacent matrix. So it is possible to check if a pair (a,b) is an edge in constant time O(1). Based on the
 * the size of the graph a permutation of 1...n ( n = number of nodes) is randomly created using the Fisher–Yates_shuffle algorithm. Then all edges (u,v) are selected for which u > v (v comes for u in the permutation).size
 * The solution is then written to the circularBuffer. In order to get the position of the write head, an additional int field is created in the shm. The supervisor prompts its generators to terminate by writing -1
 * to the write head position field. In that case the generator cleans up its resources and terminates.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char** argv) {
    while ((getopt(argc, argv, ":")) != -1 ){
        usageMessage();
    }
    
    //at least one edge
    if((argc - optind) <= 0) {
        usageMessage();
    }
    
    const int numEdges = argc - optind;
    //represents max number of node in graph -> needed for random algorithm
    int max = 0;
    //graph is represented by its edges - only temporary to create adjmat
    struct edge graph[numEdges];
        
    int index = 0;
    for(int i = optind; i < argc; i++) {
       graph[index] = parseEdge(argv[i], &max, PROG_NAME);
        index++;
    }
    
    //init adjMatrix to check if (i,j) is an edge of the graph in O(1)
    //1 if (i,j) element of edges
    int adMat[max+1][max+1];
    memset(adMat,0,(max+1)*sizeof(adMat[0]));
    for(int i = 0; i < numEdges; i++) {
        unsigned int k = graph[i].start;
        unsigned int l = graph[i].end;
        adMat[k][l] = 1;
    }
    
    //change random variable at every call
    srand(time(NULL));
    
    //fisher yates shuffle
    int nodes[max+1];
    
    //init each node
    for(int i = 0; i <= max; i++) {
        nodes[i] = i;
    }
    
    //init shared memory
    int fdBuffer;
    struct fbArc *circularBuffer;
    int *writeHead;
    fdBuffer = shm_open(shmName,O_RDWR,0600);
    
    if(fdBuffer < 0) {
        failedWithError(PROG_NAME, "failed to open shared circular buffer. Have you started the supervisor yet?", 1);
    }

    //map shared memory to virtual file -> returns starting address of circular buffer
    circularBuffer = mmap(NULL, bufferSize * sizeof(struct fbArc) + sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fdBuffer, 0);
    //init write head position
    writeHead = (int*)(circularBuffer + bufferSize);
    
    if(circularBuffer == MAP_FAILED) {
        closeSHM(fdBuffer, PROG_NAME);
        failedWithError(PROG_NAME, "failed to map circular buffer to virtual file", 1);
    }
    
    //setup semaphores
    sem_t *synchGem = sem_open(SYNCH_GEM,0);
    if(synchGem == SEM_FAILED) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 0);
        failedWithError(PROG_NAME, "failed to init synch semaphore", 1);
    }
    
    sem_t *readSem = sem_open(READ_SEM,0);
    if(readSem == SEM_FAILED) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 0);
        closeSEM(synchGem, PROG_NAME);
        failedWithError(PROG_NAME, "failed to init write semaphore", 1);
    }
    
    sem_t *writeSem = sem_open(WRITE_SEM,0);
    if(readSem == SEM_FAILED) {
        cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 0);
        closeSEM(synchGem, PROG_NAME);
        closeSEM(readSem, PROG_NAME);
        failedWithError(PROG_NAME, "failed to init read semaphore", 1);
    }
    
    //find solutions iteratively
    while(1) {
        //source: https://en.wikipedia.org/wiki/Fisher–Yates_shuffle
        for(int i = max; i > 0; i--) {
            int j = rand() % (i+1);
            swap(&nodes[i], &nodes[j]);
        }
        
        //extract feedback set from permutation
        struct fbArc solution;
        struct edge edges[8];
        memset(edges,0,8*sizeof(edges[0]));
        if(fbAlgorithm(edges, max, adMat, nodes) >= 0) {
            //report to circular buffer
            for(int i = 0; i < 8; i++) {
                solution.edges[i] = edges[i];
            }
            if(sem_wait(synchGem) < 0) {
                if(errno == EINTR) {
                    continue;
                }
                else {
                    cleanupSHM(shmName, circularBuffer, fdBuffer, PROG_NAME, 0);
                    closeSEM(synchGem, PROG_NAME);
                    closeSEM(readSem, PROG_NAME);
                    closeSEM(writeSem, PROG_NAME);
                    failedWithError(PROG_NAME, "failed to wait on synchGem", 1);
                }
            }
            int pos = *writeHead;
            if(pos < 0) {
                //stop to ocupy write head, so other generatores are able to write
                if(sem_post(synchGem) < 0) {
                    cleanupSHM(shmName,circularBuffer, fdBuffer, PROG_NAME, 0);
                    closeSEM(synchGem, PROG_NAME);
                    closeSEM(readSem, PROG_NAME);
                    closeSEM(writeSem, PROG_NAME);
                    failedWithError(PROG_NAME, "failed to post on synchGem", 1);
                }
                break;
            }
            if(sem_wait(writeSem) < 0) {
                if(errno == EINTR) {
                    continue;
                }
                else {
                    cleanupSHM(shmName,circularBuffer, fdBuffer, PROG_NAME, 0);
                    closeSEM(synchGem, PROG_NAME);
                    closeSEM(readSem, PROG_NAME);
                    closeSEM(writeSem, PROG_NAME);
                    failedWithError(PROG_NAME, "failed to wait on writeSem", 1);
                }
            }
            struct fbArc *absWriteHead = circularBuffer + pos;
            //uncomment the following line to print solutions
            //print(solution);
            *absWriteHead = solution;
            *writeHead = (pos + 1) % bufferSize;
            
            if(sem_post(readSem) < 0) {
                cleanupSHM(shmName,circularBuffer, fdBuffer, PROG_NAME, 0);
                closeSEM(synchGem, PROG_NAME);
                closeSEM(readSem, PROG_NAME);
                closeSEM(writeSem, PROG_NAME);
                failedWithError(PROG_NAME, "failed to post on readSem", 1);
            }
            if(sem_post(synchGem) < 0) {
                cleanupSHM(shmName,circularBuffer, fdBuffer, PROG_NAME, 0);
                closeSEM(synchGem, PROG_NAME);
                closeSEM(readSem, PROG_NAME);
                closeSEM(writeSem, PROG_NAME);
                failedWithError(PROG_NAME, "failed to post on synchGem", 1);
            }
        }
    }
    
    int errorOnCleanup = 0;
    //cleanup shm
    if(unmap(circularBuffer, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(closeSHM(fdBuffer, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    //cleanup semaphores
    if(closeSEM(readSem, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(closeSEM(writeSem, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(closeSEM(synchGem, PROG_NAME) < 0) {
        errorOnCleanup = -1;
    }
    if(errorOnCleanup < 0) {
        failedWithError(PROG_NAME, "failed with error on cleanup", 1);
    }
    return EXIT_SUCCESS;
}

