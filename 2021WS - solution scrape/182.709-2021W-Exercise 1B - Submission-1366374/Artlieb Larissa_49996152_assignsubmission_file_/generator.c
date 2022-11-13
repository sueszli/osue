/** @file generator.c
* @author Larissa Artlieb 
* @date 13.11.2021
* @brief Implements the functionality required in Uebung 1B
* @details 	This module implements the generator side of Feedback Arc Set */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
 #include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include "supervisor.h"
#include "generator.h"

sem_t  *freeSpaceSem, *usedSpaceSem, *writePosSem;

struct circular_buffer *buffer;
volatile sig_atomic_t quit = 0;


/**
 * @brief handles interrupts
 * @details sets the global quit variable to 1 when the program is interrupted by SIGINT or SIGTERM
 * @param signal number
 */
void handle_signal(int signal) {
    quit = 1;
}

/**
 * @brief Write candidates to buffer
 * @details Write the generated candidates to the circular buffer in shared memory
 * @param graph
 * @param length
 */
void writeToBuffer(int *graph, int length)
{
    int wr_pos = buffer->wr_pos;

    // store candidate set
    for (int i=0; i<length; i++) {
        for (int j=0; j<NODES_PER_EDGE; j++) {
            buffer->candidateSets[wr_pos][i][j] = *((graph + i * NODES_PER_EDGE) + j);
        }
        printf("Edge %i-%i\n", buffer->candidateSets[wr_pos][i][0], buffer->candidateSets[wr_pos][i][1]);
    }

    // store number of edges
    buffer->numberOfEdges[wr_pos] = length;
    printf("sent candidateSets with %i edges\n", length);
}



/**
 * @brief Main generator program for feedback arc sets
 * @details Generates candidate sets to store them into the circular buffer
 * @param argc argument count
 * @param argv argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
int main (int argc, char *argv[])
{
    if (argc <= 1) {
        // not enough arguments
        printf("[%s] Usage: ./generator EDGE1...\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand(time(NULL)); // initialize randomness


    // read positional arguments
    char graph[argc][2];
    for(int i = 1; i < argc; i++) {
        graph[i-1][0] = argv[i][0];
        graph[i-1][1] = argv[i][2];
    }


    // set up signal (interrupt) handler
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    // initialize shared memory
    int fd_shm; 
    fd_shm = shm_open(SHM_NAME, O_RDWR, 0600);
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);


    // OPEN SEMAPHORES
    freeSpaceSem = sem_open(FREESPACESEM_NAME, O_CREAT, 0600, sizeof(struct circular_buffer)); //initialized to size of buffer
    usedSpaceSem = sem_open(USEDSPACESEM_NAME, O_CREAT, 0600, 0); //initialized to 0
    writePosSem = sem_open(WRITEPOSSEM_NAME, O_CREAT, 0600, 1); //initialized to 1



    while(buffer->stop == 0 && quit == 0) {
        
        //calculate random solution
        int candidateSet[argc][2];
        memset(candidateSet, 0, sizeof(candidateSet));

        char count = 0; // maximum node
        for(int i = 0; i < (argc -1); i++) {
            if(graph[i][0] > count) {
                count = graph[i][0];
            }

            if(graph[i][1] > count) {
                count = graph[i][1];
            }
        } 

        // maximum node as integer
        int maxNode = (count-'0')+1;
        
        int randomizedNodes[maxNode];
        memset(randomizedNodes, -1, sizeof(randomizedNodes)); // initialize all fields with -1


        int nodes[maxNode];
        for(int i = 0; i < maxNode; i++) {
            nodes[i] = i; 
        }


        for(int i = 0; i < maxNode; i++){
            int r = rand() % maxNode; // random integer between 0 and argc.

            if(randomizedNodes[r] == -1) {
                randomizedNodes[r] = nodes[i];
            } else {
                i--;
                continue;
            }
        }

        // debug output (randomized nodes)
        /*
        for(int i = 1; i < maxNode; i++) {
            printf("RANDOMIZED %i \n", randomizedNodes[i]);
        }
        */

        //erase cycles
        int candidateIndex = 0; 

        // search start and end index of each edge
        for(int i = 0; i < (argc-1); i++){
            int from = (graph[i][0]-'0'); 
            int fromAt = 0;
            int to = (graph[i][1]-'0'); 
            int toAt = 0; 

            //looks up index of start node in randomized nodes
            for(int j = 0; j < maxNode; j++) {
                if(randomizedNodes[j] == from) {
                    fromAt = j-1;
                    //printf("FROM : %i \n", from); // debug output
                    //printf("FROM AT:  %i \n", fromAt);
                }
            }

            //looks up index of end node in randomized nodes
            for(int k = 0; k < maxNode; k++) {
                if(randomizedNodes[k] == to) {
                    toAt = k-1;
                    //printf("TO : %i \n", to); // debug output
                    //printf("TO AT:  %i \n", toAt);
                }
            }

            //checks if edge is eligible
            if(fromAt > toAt) {
                candidateSet[candidateIndex][0] = from;
                candidateSet[candidateIndex++][1] = to; 
            }

        }



        // ------- SYNCHRONIZATION WITH SEMAPHORES START -------
        if (sem_wait(freeSpaceSem) == -1) {
            if (errno == EINTR) { // interrupted by signal
                quit = 0; // we want to return EXIT_SUCCESS below
                break;
            }
            break; // we want to return EXIT_FAILURE below
        }
        if (sem_wait(writePosSem) == -1) {
            if (errno == EINTR) { // interrupted by signal
                quit = 0; // we want to return EXIT_SUCCESS below
                break;
            }
            break; // we want to return EXIT_FAILURE below
        }

        //------ CRITICAL SECTION START ------
        writeToBuffer(*candidateSet, candidateIndex);
        buffer->wr_pos += 1;
        buffer->wr_pos %= MAX_DATA_SETS;
        //------ CRITICAL SECTION END ------


        sem_post(writePosSem);
        sem_post(usedSpaceSem);
        // ------- SYNCHRONIZATION WITH SEMAPHORES END -------


        fflush(stdout);
        //sleep(5); // for testing
    }


    // cleanup and close shared memory
    if(munmap(buffer, sizeof(*buffer)) != 0) {
        fprintf(stderr, "%s - unmapping failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    } 

    if(close(fd_shm) != 0) {
        fprintf(stderr, "%s - closing shm failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    // close semaphores
    if(sem_close(freeSpaceSem) != 0) {
        fprintf(stderr, "%s - closing freeSpaceSem failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }  

    if(sem_close(usedSpaceSem) != 0) {
        fprintf(stderr, "%s - closing usedSpaceSem failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    if(sem_close(writePosSem) != 0) {
        fprintf(stderr, "%s - closing writePosSem failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    // return with correct status code
    if (quit == 1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS; 
}
