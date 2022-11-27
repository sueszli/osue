#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include "supervisor.h"
#include <getopt.h>

/**
 * @name: generator
 * @author: Moritz Hammerer, 11902102
 * @date: 08.11.2021
 * 
 * @brief Checks how many edges have to be removed to make a graph 3 colorable
 *
 * @details Randomizes the colors of the vertices and checks how many edges would have to
 * be removed to make the graph 3colorable. If a smaller solution than that in the shared memory
 * is found it gets published, otherwise the loop starts again.
 * 
 * @param "int-int" any number of Arguments in this format showing the edges
 * 
 */


volatile sig_atomic_t quit = 0;
char *myprog;

/**
 * @brief Acts as Signalhandler and sets quit to one
 *  
 * @param signal    unused argument
 */
void handle_signal(int signal)
{
    quit = 1;
}

/**
 * @brief Gives out custom error message followed by errno and exits program
 * 
 * @details Takes an Char* with changable text and outputs it together with the name of the program and the corresponding errno.
 * Afterwards the program gets closed with ERROR_FAILURE
 *  
 * @param text      Char* Short text to futher explain the problem
 */
void errorOut(char* text){
    fprintf(stderr, "[%s] %s: %s\n", myprog, text, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Creating Shared Memory and Semaphores for generators and output each shorter answer
 *  
 * @details Randomizes the colors of the vertices and checks how many edges would have to
 * be removed to make the graph 3colorable. If a smaller solution than that in the shared memory
 * is found it gets published, otherwise the loop starts again.
 */
int main(int argc, char* argv[]) {
    myprog = argv[0];
    if (argc == 1){
        errno = EINVAL;
        errorOut("Usage error. Has to be started with at least one edge in format 'u-v'");
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //Starting
    
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1){
        errorOut("Opening of '/shm11902102' failed");
    }

    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED){
        errorOut("Mapping of '/shm11902102' failed");
    }

    sem_t *writeSem = sem_open(WRITESEM, 0);
    sem_t *readSem = sem_open(READSEM, 0); 
    sem_t *permSem = sem_open(PERMSEM, 0); 

    if (writeSem == SEM_FAILED || readSem == SEM_FAILED || permSem == SEM_FAILED )
    {
        errorOut("Opening of Semaphores failed");
    }
    
    //critical
    while (!quit) {
        int argcount = argc-optind;

        struct edge workEdges[argcount];
        int workEdgeCount = 0;
        int vertexCount = 0;

        //Get all edges
        for (int i = 1; i < argc; i++)
        {
            struct edge tempEdge;

            char* ptr;
            int vertexU =  strtol(argv[i], &ptr, 10);
            ptr++;
            int vertexV =  strtol(ptr, &ptr, 10);
            
            tempEdge.u = vertexU;
            tempEdge.v = vertexV;

            workEdges[i-1] = tempEdge;
            workEdgeCount++;

            //Number of Vertices by name
            if (vertexU > vertexCount){
                vertexCount = vertexU + 1;
            }
            if (vertexV > vertexCount){
                vertexCount = vertexV + 1;
            }
        }
        
        //Randomize
        int coloredVertices[vertexCount];
        for (int i = 0; i < vertexCount; i++){
            coloredVertices[i] = rand() % 3;
        }

        //Check all workEdges
        struct dataEntry val;
        int valCount = 0;
        int highestAllowed = myshm->currentMin;
        for (int i = 0; i < workEdgeCount; i++)
        {
            struct edge tempEdge = workEdges[i];

            int colorU = coloredVertices[tempEdge.u];
            int colorV = coloredVertices[tempEdge.v];

            if (colorU == colorV){
                valCount++;

                if (valCount <= highestAllowed)
                {
                    val.edges[valCount-1] = tempEdge;
                } else{
                    valCount = highestAllowed + 1; //Setting count high so it gets discarded
                    break;
                }
            }
        }

        val.count = valCount;
        

        if (val.count >= highestAllowed){
            //fprintf(stderr, "[%s] Discarded. Too big: %i\n", myprog, val.count);
            if (myshm->state == 0){
                quit = 1;
            }
        }else{
            sem_wait(permSem); //Wait for write-permission
            if (myshm->state == 0){
                quit = 1;
            }
            sem_wait(writeSem);

            myshm->data[myshm->wr_pos] = val;

            int wr_pos = myshm->wr_pos + 1;
            wr_pos %= BUFFER_SIZE;
            myshm->wr_pos = wr_pos;
            
            sem_post(readSem);
            sem_post(permSem); //Give back write-permission
        }
    }
    
    //Cleanup
    sem_close(writeSem);
    sem_close(readSem);
    sem_close(permSem);

    exit(EXIT_SUCCESS);
}