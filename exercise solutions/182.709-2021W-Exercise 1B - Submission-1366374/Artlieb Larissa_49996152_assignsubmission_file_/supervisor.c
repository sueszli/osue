/** @file supervisor.c
* @author Larissa Artlieb 
* @date 13.11.2021
* @brief Implements the functionality required in Uebung 1B
* @details 	This module implements the supervisor side of Feedback Arc Set */

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
#include <limits.h>
#include "supervisor.h"

/**
* @brief implements supervisor
* @details sets up shared memory and the semaphores and initializes the circular buffe, then reads from the circular buffer and decides on the best solution
* @param argc argument count
* @param argv argument vector
* @return EXIT_SUCCESS on success and EXIT_FAILURE otherwise
*/

sem_t  *freeSpaceSem, *usedSpaceSem, *writePosSem; ///< @var semaphores
struct circular_buffer *buffer; ///< @var shared circular buffer
int minimalSet[MAX_EDGES][NODES_PER_EDGE]; ///< @var current minimum set
int minimalNumberOfEdges = INT_MAX; ///< @var current minimal number of edges
int rd_pos = 0; ///< @var read position of buffer
volatile sig_atomic_t quit = 0; ///< @var quit flag, set when interrupt occurs


/**
 * @brief handles interrupts
 * @details sets the global quit variable to 1 when the program is interrupted by SIGINT or SIGTERM
 * @param signal number
 */
void handle_signal(int signal) {
    quit = 1;
}



/**
 * @brief Read candidate sets from circular buffer
 * @detail Read candidate sets from shared memory and selects the minimum feedback arc set
 * @param progname
 */
void readFromBuffer(char* progname) {
    if (sem_wait(usedSpaceSem) == -1) {
        if (errno == EINTR) { // interrupted by signal
            return;
        }
        quit = 1;
        return;
    }

    if (buffer->numberOfEdges[rd_pos] == 0) {
        printf("[%s] The graph is acyclic!\n", progname);
        quit = 1;
        return;
    }

    // printf("received candidateSets with %i edges:\n", (buffer->numberOfEdges[rd_pos]));
    if (buffer->numberOfEdges[rd_pos] < minimalNumberOfEdges) {
        // new minimal set detected

        minimalNumberOfEdges = buffer->numberOfEdges[rd_pos]; // overwrite current minimum
        printf("[%s] Solution with %i edges: ", progname, buffer->numberOfEdges[rd_pos]);

        for(int i = 0; i < (buffer->numberOfEdges[rd_pos]); i++) {
            // store all edges into minimalSet
            minimalSet[i][0] = buffer->candidateSets[rd_pos][i][0];
            minimalSet[i][1] = buffer->candidateSets[rd_pos][i][1];
            // print all edges
            printf("%i-%i ", buffer->candidateSets[rd_pos][i][0], buffer->candidateSets[rd_pos][i][1]);
        }

        printf("\n");
    }



    sem_post(freeSpaceSem);

    rd_pos += 1;
    rd_pos %= MAX_DATA_SETS;
}


/**
 * @brief Main supervisor program for feedback arc sets
 * @details Receives candidate sets from circular buffer and selects the minimum feedback arc sets
 * @param argc argument count
 * @param argv argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
int main (int argc, char *argv[])
{
    int fd_shm;

    // set up signal (interrupt) handler
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    // was needed on apple silicon processors:
    //shm_unlink(SHM_NAME);


    // initialize shared memory
    fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);

    if(fd_shm == -1) {
        fprintf(stderr, "%s - open failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    } 

    if(ftruncate(fd_shm, sizeof(struct circular_buffer)) < 0) {
        fprintf(stderr, "%s - ftruncate failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);

    if(buffer == MAP_FAILED) {
        fprintf(stderr, "%s - mapping failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }


    // OPEN SEMAPHORES
    freeSpaceSem = sem_open(FREESPACESEM_NAME, O_CREAT, 0600, sizeof(struct circular_buffer)); //initialized to size of buffer
    usedSpaceSem = sem_open(USEDSPACESEM_NAME, O_CREAT, 0600, 0); //initialized to 0
    writePosSem = sem_open(WRITEPOSSEM_NAME, O_CREAT, 0600, 1); //initialized to 1

    // initialize shared memory
    buffer->stop = 0;
    buffer->wr_pos = 0;


    // read from shared memory
    while(buffer->stop == 0) {
        if (quit == 1) {
            buffer->stop = 1; // tell generators to stop
            continue;
        }

        //------ CRITICAL SECTION START ------
        readFromBuffer(argv[0]);
        //------ CRITICAL SECTION END ------

        fflush(stdout);
    }



    // cleanup and remove shared memory
    if(munmap(buffer, sizeof(*buffer)) != 0) {
        fprintf(stderr, "%s - unmapping failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    } 

    if(close(fd_shm) != 0) {
        fprintf(stderr, "%s - closing shm failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    if(shm_unlink(SHM_NAME) != 0) {
        fprintf(stderr, "%s - unlinking failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    // cleanup and remove semaphores
    if(sem_close(freeSpaceSem) != 0) {
        fprintf(stderr, "%s - closing freeSpaceSem failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }  

    if(sem_close(usedSpaceSem) != 0) {
        fprintf(stderr, "%s - closing usedSpaceSem failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }  

    if(sem_unlink(FREESPACESEM_NAME) != 0 ) {
        fprintf(stderr, "%s - unlinking freeSpaceSem failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    if(sem_unlink(USEDSPACESEM_NAME) != 0 ) {
        fprintf(stderr, "%s - unlinking usedSpaceSem failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    if(sem_unlink(WRITEPOSSEM_NAME) != 0 ) {
        fprintf(stderr, "%s - unlinking writePosSem failed: %s", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; 
}
